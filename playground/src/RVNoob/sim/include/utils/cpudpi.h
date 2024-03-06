#ifndef __UTILS_CPUDPI_H__
#define __UTILS_CPUDPI_H__

#include "common.h"

void init_difftest(char *ref_so_file, long img_size, int port, void *cpu);
void difftest_step(vaddr_t pc, vaddr_t npc);
void refresh_gpr_pc_csr();
void difftest_skip_ref(vaddr_t addr);

extern char *diff_file;
extern uint32_t cpu_inst;
extern uint32_t cpu_pc, cpu_npc, cpu_inst_cnt;
extern uint8_t diff_en;
extern uint32_t mem_pc;
extern uint32_t diff_pc, diff_inst;
extern uint8_t wb_valid;
extern vaddr_t wb_pc;
extern uint32_t wb_inst;
#endif