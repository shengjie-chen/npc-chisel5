#ifndef __TRACE_TRACE_H__
#define __TRACE_TRACE_H__
#include "common.h"

void init_ftrace(const char *elf_file);
void ftrace_call_ret(uint32_t cpu_inst, vaddr_t pc, vaddr_t npc);
void device_update();

#ifdef CONFIG_MTRACE
extern const char *mtrace_file;
extern FILE *mtrace_fp;
#endif

#ifdef CONFIG_FTRACE
extern const char *ftrace_file;
extern FILE *ftrace_fp;
extern char *elf_file;
#endif

#ifdef CONFIG_ITRACE
extern char logbuf[128];
extern FILE *itrace_fp;
extern const char *itrace_file;
#endif

#endif