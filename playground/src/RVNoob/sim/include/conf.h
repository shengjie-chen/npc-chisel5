/*
 * @Author: Shengjie Chen chenshengjie1999@126.com
 * @Date: 2022-11-05 16:32:16
 * @LastEditors: Shengjie Chen chenshengjie1999@126.com
 * @LastEditTime: 2022-12-10 10:41:57
 * @FilePath: /npc/playground/src/RVnpc/RVNoob/conf.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __MACRO_CONF_H__
#define __MACRO_CONF_H__

// ---------------------------->switch
#define CONFIG_FSTWAVE
#define SOC_SIM

// #define CONFIG_ITRACE
// #define CONFIG_FTRACE
// #define CONFIG_MTRACE

#define SPMU_ENABLE

#ifdef SPMU_ENABLE
// #define RAS_SPMU
#endif

// #define CONFIG_DIFFTEST
// #define CONFIG_DIFFTEST_REF_MEM_POINT

// #define SIM_TIME_MAX 100000

// #define CONFIG_DUMPWAVE
// #define CONFIG_DUMPSTART 0
// #define CONFIG_DUMPSTART 40000000













// ---------------------------->switch end

#ifndef SIM_TIME_MAX
#define SIM_TIME_MAX -1
#endif

#if defined(CONFIG_ITRACE) || defined(RAS_SPMU) || defined(CONFIG_FTRACE)
#ifndef CONFIG_DUMPWAVE
#define CONFIG_DUMPSTART 0
#endif
#endif

// #define BIGPROGRAM

// #ifdef BIGPROGRAM
// #define BIGPROGRAMDEBUG
////#define CONFIG_DIFFTEST
// #endif

// #ifndef BIGPROGRAM
// #define CONFIG_ITRACE
// #define CONFIG_FTRACE
// #define CONFIG_MTRACE
// #define CONFIG_DUMPWAVE
////#define CONFIG_DIFFTEST
// #define CONFIG_DUMPSTART 0
// #endif

// #ifdef BIGPROGRAMDEBUG
// // #define CONFIG_MTRACE
// #define CONFIG_ITRACE
// #define CONFIG_FTRACE
// #define CONFIG_DUMPWAVE
// #define CONFIG_DUMPSTART 0
// // #define CONFIG_DUMPSTART 11200000
// #endif

#define CONFIG_ISA64 1
#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000

#define CONFIG_VGA_SHOW_SCREEN 1
#define CONFIG_VGA_SIZE_400x300 1

#endif