/*
 * @Author: Shengjie Chen chenshengjie1999@126.com
 * @Date: 2022-12-07 22:51:47
 * @LastEditors: Shengjie Chen chenshengjie1999@126.com
 * @LastEditTime: 2022-12-10 10:48:32
 * @FilePath: /npc/playground/src/RVnpc/RVNoob/useddpi.c
 * @Description: 用到的dpi变量和函数集合
 */

#include "common.h"

/// @brief dpi函数用于读任意有效地址
/// @param raddr
/// @param rdata
extern "C" void pmem_read_dpi(long long raddr, long long *rdata, long long pc) {
    if (raddr == RTC_ADDR) {
        struct timeval now;
        gettimeofday(&now, NULL);
        *rdata = now.tv_sec * 1000000 + now.tv_usec;
#ifdef CONFIG_MTRACE
        fprintf(mtrace_fp, "read  rtc ## addr: %llx", raddr & ~0x7ull);
        fprintf(mtrace_fp, " -> 0x%016llx \n", *rdata);
#endif
#ifdef CONFIG_DIFFTEST
        difftest_skip_ref(pc);
#endif
        return;
    }

    if (raddr == KBD_ADDR) {
        i8042_data_io_handler();
        *rdata = i8042_data_port_base;
#ifdef CONFIG_MTRACE
        fprintf(mtrace_fp, "read  keyboard ## addr: %llx", raddr & ~0x7ull);
        fprintf(mtrace_fp, " -> 0x%08llx \n", *rdata);
#endif
#ifdef CONFIG_DIFFTEST
        difftest_skip_ref(pc);
#endif
        return;
    }

    if (raddr == VGACTL_ADDR || raddr == (VGACTL_ADDR + 2)) {
        *rdata = vgactl_port_base;
#ifdef CONFIG_MTRACE
        fprintf(mtrace_fp, "read  vgactrl ## addr: %llx", raddr & ~0x7ull);
        fprintf(mtrace_fp, " -> 0x%08llx \n", *rdata);
#endif
#ifdef CONFIG_DIFFTEST
        difftest_skip_ref(pc);
#endif
        return;
    }

    if (raddr >= FB_ADDR && raddr < (FB_ADDR + screen_size)) {
        *rdata = *(uint32_t *)((uint8_t *)vmem + raddr - FB_ADDR);
        // *rdata = *(uint32_t *)((uint8_t *)vmem + raddr - FB_ADDR);
#ifdef CONFIG_DIFFTEST
        difftest_skip_ref(pc);
#endif
        return;
    }

    // 总是读取地址为`raddr & ~0x7ull`的8字节返回给`rdata`
    if (likely(in_pmem(raddr))) {
        *rdata = pmem_read(raddr & ~0x7ull, 8);
#ifdef CONFIG_MTRACE
        fprintf(mtrace_fp, "T:%ld\tread  pmem ## addr: %llx", main_time, raddr & ~0x7ull);
        fprintf(mtrace_fp, " -> 0x%016llx \n", *rdata);
#endif
    }
}

/// @brief dpi函数用于写任意有效地址
/// @param waddr
/// @param wdata
/// @param wmask
extern "C" void pmem_write_dpi(long long waddr, long long wdata, char wmask, long long pc) {
    // 总是往地址为`waddr & ~0x7ull`的8字节按写掩码`wmask`写入`wdata`
    // `wmask`中每比特表示`wdata`中1个字节的掩码,
    // 如`wmask = 0x3`代表只写入最低2个字节, 内存中的其它字节保持不变
    // printf("waddr is %016x\n",waddr);
    if (waddr == SERIAL_PORT) {
        printf("%c", (char)wdata);
#ifdef CONFIG_MTRACE
        fprintf(mtrace_fp, "write serial ## addr: %llx", waddr & ~0x7ull);
        fprintf(mtrace_fp, " -> 0x%016llx ", wdata);
        fprintf(mtrace_fp, " wmask-> 0x%02x \n", (uint8_t)wmask);
#endif
#ifdef CONFIG_DIFFTEST
        difftest_skip_ref(pc);
#endif
        return;
    }

    if (waddr >= FB_ADDR && waddr < (FB_ADDR + screen_size)) {
        // printf("wmask = %x\n", wmask);
        assert((wmask == 0x0f) || (wmask == (char)0xf0));
        if (wmask == 0x0f) {
            *(uint32_t *)((uint8_t *)vmem + (waddr & ~0x7ull) - FB_ADDR) = wdata;
        } else if (wmask == (char)0xf0) {
            *(uint32_t *)((uint8_t *)vmem + (waddr & ~0x7ull) - FB_ADDR + 4) = wdata >> 32;
        }
#ifdef CONFIG_DIFFTEST
        difftest_skip_ref(pc);
#endif
        return;
    }

    if (waddr == VGACTL_ADDR + 4) {
        assert(wmask == (char)0xf0);
        //    if(wmask == (char)0x0f)
        vgactl_port_base_syn = wdata >> 32;
#ifdef CONFIG_DIFFTEST
        difftest_skip_ref(pc);
#endif
        return;
    }

    if (waddr < CONFIG_MBASE || waddr >= (CONFIG_MBASE + CONFIG_MSIZE)) {
        printf("!!! sim_time:%ld, out of bound. write addr:%llx\n", main_time, waddr);
        npc_state.state = NPC_ABORT;
        return;
    }

#ifdef CONFIG_MTRACE
    fprintf(mtrace_fp, "T:%ld\twrite pmem ## addr: %llx", main_time, waddr & ~0x7ull);
    fprintf(mtrace_fp, " -> 0x%016llx ", wdata);
    fprintf(mtrace_fp, " wmask-> 0x%02x \n", (uint8_t)wmask);
#endif
    for (int i = 0; i < 8; i++) {
        if ((wmask >> i) & 1 == 1) {
            pmem_write((waddr & ~0x7ull) + i, 1, wdata >> (8 * i));
        }
    }
}

/// @brief 获取cpu的指令，当前得到idu的指令
uint32_t cpu_inst;
extern "C" void inst_change(const svLogicVecVal *r) {
    cpu_inst = *(uint32_t *)(r);
    // printf("inst : %x\n", cpu_inst);
}

/// @brief 获取pc值，即当周期pc寄存器的值; 获取下一个pc值，即下一周期pc寄存器的值; 获取当前执行了多少指令
uint32_t cpu_pc, cpu_npc, cpu_inst_cnt;
extern "C" void pc_change(const svLogicVecVal *a, const svLogicVecVal *b, const svLogicVecVal *c) {
    cpu_pc = *(vaddr_t *)(a);
    cpu_npc = *(vaddr_t *)(b);
    cpu_inst_cnt = *(uint64_t *)(c);
}

/// @brief 获取pc值，即当周期pc寄存器的值; 获取下一个pc值，即下一周期pc寄存器的值; 获取当前执行了多少指令
uint8_t diff_en;
uint32_t diff_pc, diff_inst;
extern "C" void difftest_change(svLogic a, const svLogicVecVal *b, const svLogicVecVal *c) {
    diff_en = a;
    diff_pc = *(vaddr_t *)(b);
    diff_inst = *(uint32_t *)(c);
}

void npc_ebreak() {
    npc_state.state = NPC_END;
    printf("!!!!!! npc ebreak !!!!!!\n");
}

// uint32_t *cpu_inst = NULL;
// extern "C" void set_inst_ptr(const svLogicVecVal *r)
//{
//   cpu_inst = (uint32_t *)(r);
// }

/// @brief 获取mem_reg.out.pc
uint32_t mem_pc;
extern "C" void mem_pc_change(const svLogicVecVal *r) { mem_pc = *(vaddr_t *)(r); }

uint8_t wb_valid;
vaddr_t wb_pc;
uint32_t wb_inst;
extern "C" void wb_change(svLogic v, const svLogicVecVal *p, const svLogicVecVal *i) {
    wb_valid = v;
    wb_pc = *(vaddr_t *)(p);
    wb_inst = *(uint32_t *)(i);
}

/// @brief 软件性能监视SPMU（Soft Performence Monitor Unit）使能，增加cache miss概率检测
uint64_t icache_hit = 0, icache_miss = 0;
extern "C" void icache_access(svLogic miss) {
    if (miss) {
        icache_miss++;
    } else {
        icache_hit++;
    }
}

uint64_t dcache_hit = 0, dcache_miss = 0;
extern "C" void dcache_access(svLogic miss) {
    if (miss) {
        dcache_miss++;
    } else {
        dcache_hit++;
    }
}

uint64_t id_branch_error = 0;
extern "C" void find_id_branch_error() { id_branch_error++; }

uint64_t exe_branch_error = 0;
extern "C" void find_exe_branch_error() { exe_branch_error++; }

uint64_t mem_branch_error = 0;
extern "C" void find_mem_branch_error() { mem_branch_error++; }

#define BR_CALL 0
#define BR_RET 1
#define BR_TAKEN 2
#define BR_TYPEB 3
#define BR_NOT 4

uint64_t branch_inst = 0;
uint8_t br_type;
uint8_t pre_taken;
uint32_t pre_target;
uint8_t true_taken;
uint32_t true_target;

uint64_t typeb_br = 0;
uint64_t ret_inst = 0;
uint64_t ret_error_inst = 0;
#ifdef RAS_SPMU
#define RAS_PATH NPC_HOME "/build/RVNoob/npc-ras-log.txt"
const char *ras_file = RAS_PATH;
FILE *ras_fp = NULL;

class RASStream {
  public:
    // 向流中添加一个位
    void addBit(bool bit) { bits.push_back(bit); }

    // 在程序结束前打印所有位
    ~RASStream() {
        ras_fp = fopen(ras_file, "w");
        if (ras_fp) {
            fprintf(ras_fp, "RASStream pre result: \n");
            for (int i = 0; i < bits.size(); i++) {
                if (i % 10 == 0) {
                    fprintf(ras_fp, "%d ~ %d: ", i, i + 9);
                }
                fprintf(ras_fp, "%d", (bool)bits[i]);
                if (i % 10 == 9) {
                    fprintf(ras_fp, "\n");
                }
            }
            fclose(ras_fp);

        } else {
            std::cout << "Unable to open file" << std::endl;
        }
    }

  private:
    std::vector<bool> bits;
};

RASStream ras_stream;
#endif

extern "C" void br_change(const svLogicVecVal *br_type_t, svLogic pre_taken_t, const svLogicVecVal *pre_target_t,
                          svLogic true_taken_t, const svLogicVecVal *true_target_t) {
    {
        branch_inst++;
        br_type = *(uint8_t *)(br_type_t);
        pre_taken = pre_taken_t;
        pre_target = *(uint32_t *)(pre_target_t);
        true_taken = true_taken_t;
        true_target = *(uint32_t *)(true_target_t);

        if (br_type == BR_RET) {
            ret_inst++;
            if (pre_target != true_target || pre_taken == 0) {
                ret_error_inst++;
#ifdef RAS_SPMU
                if (main_time > CONFIG_DUMPSTART) {
                    ras_stream.addBit(0);
                }
#endif
            } else {
#ifdef RAS_SPMU
                if (main_time > CONFIG_DUMPSTART) {
                    ras_stream.addBit(1);
                }
#endif
            }
        }

        if (br_type == BR_TYPEB) {
            typeb_br++;
        }
    }
}