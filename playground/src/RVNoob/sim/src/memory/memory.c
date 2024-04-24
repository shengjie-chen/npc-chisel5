#include "memory/memory.h"
#include "common.h"
#include "device/device.h"
#include "trace/trace.h"
#include "utils/difftest.h"

extern vluint64_t main_time;

char *img_file = NULL;
uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

bool in_pmem(paddr_t addr) { return (addr >= CONFIG_MBASE) && (addr - CONFIG_MSIZE < (paddr_t)CONFIG_MBASE); }
#ifdef SOC_SIM
static inline bool in_mrom(paddr_t addr) { return (addr >= MROM_PORT) && (addr - MROM_SIZE < (paddr_t)MROM_PORT); }
static inline bool in_sram(paddr_t addr) { return (addr >= SRAM_PORT) && (addr - SRAM_SIZE < (paddr_t)SRAM_PORT); }
#endif

/// @brief 将客户地址向主机地址转换-----地址可以分为客户地址（npc地址，通常是0x800...）和主机地址（实际地址）
/// @param paddr
/// @return
uint8_t *guest_to_host(paddr_t paddr) {
#ifdef SOC_SIM
    if (likely(in_mrom(paddr)))
        return mrom + paddr - MROM_PORT;
    else
        Assert(0, "paddr = %x, out of bound!\n", paddr);
#else
    return pmem + paddr - CONFIG_MBASE;
#endif
}

long load_img() {
    if (~strcmp(img_file, "default")) {
        printf("No image is given. Use the default build-in image.\n");
        memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));
        return 4096; // built-in image size
    }

    FILE *fp = fopen(img_file, "rb");
    if (fp == NULL) {
        printf("Can not open %s\n", img_file);
        return 1;
    } else {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);

        printf("The image is %s, size = %ld\n", img_file, size);

        fseek(fp, 0, SEEK_SET);
        int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
        assert(ret == 1);

        fclose(fp);
        return size;
    }
}

word_t host_read(void *addr, int len) {
    switch (len) {
    case 1:
        return *(uint8_t *)addr;
    case 2:
        return *(uint16_t *)addr;
    case 4:
        return *(uint32_t *)addr;
        IFDEF(CONFIG_ISA64, case 8 : return *(uint64_t *)addr);
    default:
        MUXDEF(CONFIG_RT_CHECK, assert(0), return 0);
    }
}

void host_write(void *addr, int len, word_t data) {
    switch (len) {
    case 1:
        *(uint8_t *)addr = data;
        return;
    case 2:
        *(uint16_t *)addr = data;
        return;
    case 4:
        *(uint32_t *)addr = data;
        return;
        IFDEF(CONFIG_ISA64, case 8 : *(uint64_t *)addr = data; return);
        IFDEF(CONFIG_RT_CHECK, default : assert(0));
    }
}

word_t pmem_read(paddr_t addr, int len) {
    word_t ret = host_read(guest_to_host(addr), len);
    return ret;
}

void pmem_write(paddr_t addr, int len, word_t data) { host_write(guest_to_host(addr), len, data); }

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