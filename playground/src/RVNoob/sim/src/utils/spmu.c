#include "common.h"

extern vluint64_t main_time;

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