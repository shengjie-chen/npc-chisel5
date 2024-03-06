#ifndef __RVNOOB_H__
#define __RVNOOB_H__
#include "conf.h"
// #if CONFIG_MBASE + CONFIG_MSIZE > 0x100000000ul
// #define PMEM64 1
// #endif

#define RESET_VECTOR (CONFIG_MBASE + CONFIG_PC_RESET_OFFSET)
#define CONFIG_PC_RESET_OFFSET 0x0

typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;
typedef MUXDEF(CONFIG_ISA64, int64_t, int32_t) sword_t;
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016lx", "0x%08x")

typedef word_t vaddr_t;
// typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;

typedef uint64_t paddr_t;

#define FMT_PADDR MUXDEF(PMEM64, "0x%016lx", "0x%08x")
typedef uint16_t ioaddr_t;

#define CONFIG_RT_CHECK 1

/// @brief NPC当前运行状态等
typedef struct {
    int state;
    vaddr_t halt_pc;
    // uint32_t halt_ret;
} NPCState;

#define CSR_NUM 4
/// @brief RV64CPU寄存器状态
typedef struct {
    word_t gpr[32];
    vaddr_t pc;
    word_t csr[CSR_NUM];
} NPC_riscv64_CPU_state;

typedef NPC_riscv64_CPU_state CPU_state;

enum { NPC_RUNNING, NPC_STOP, NPC_END, NPC_ABORT, NPC_QUIT };

extern NPCState npc_state;

#endif