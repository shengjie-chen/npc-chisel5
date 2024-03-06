#ifndef __UTILS_DIFFTEST_H__
#define __UTILS_DIFFTEST_H__

#include "common.h"

void init_difftest(char *ref_so_file, long img_size, int port, void *cpu);
void difftest_step(vaddr_t pc, vaddr_t npc);
void refresh_gpr_pc_csr();
void difftest_skip_ref(vaddr_t addr);

extern char *diff_file;

#endif