#ifndef __DEVICE_DEVICE_H__
#define __DEVICE_DEVICE_H__
#include "common.h"
#include <SDL2/SDL.h>

// // for AM IOE
// #define io_read(reg) \
//     ({ \
//         reg##_T __io_param; \
//         ioe_read(reg, &__io_param); \
//         __io_param; \
//     })

// #define io_write(reg, ...)                                                                                             \
//     ({                                                                                                                 \
//         reg##_T __io_param = (reg##_T){__VA_ARGS__};                                                                   \
//         ioe_write(reg, &__io_param);                                                                                   \
//     })

#ifdef SOC_SIM

#define SRAM_PORT 0x0f000000
#define UART_PORT 0x10000000L
#define MROM_PORT 0x20000000
#define FLASH_PORT 0x30000000

#define SRAM_SIZE 8 * 1024
#define MROM_SIZE 4 * 1024
#define FLASH_SIZE 4 * 1024

#define DEVICE_BASE 0xa0000000
#define MMIO_BASE DEVICE_BASE

#define SERIAL_PORT (DEVICE_BASE + 0x00003f8)
#define KBD_ADDR (DEVICE_BASE + 0x0000060)
#define RTC_ADDR (DEVICE_BASE + 0x0000048)
#define VGACTL_ADDR (DEVICE_BASE + 0x0000100)
#define AUDIO_ADDR (DEVICE_BASE + 0x0000200)
#define DISK_ADDR (DEVICE_BASE + 0x0000300)
#define FB_ADDR (MMIO_BASE + 0x1000000)
#define AUDIO_SBUF_ADDR (MMIO_BASE + 0x1200000)

#else
#define DEVICE_BASE 0xa0000000
#define MMIO_BASE DEVICE_BASE

#define SERIAL_PORT (DEVICE_BASE + 0x00003f8)
#define KBD_ADDR (DEVICE_BASE + 0x0000060)
#define RTC_ADDR (DEVICE_BASE + 0x0000048)
#define VGACTL_ADDR (DEVICE_BASE + 0x0000100)
#define AUDIO_ADDR (DEVICE_BASE + 0x0000200)
#define DISK_ADDR (DEVICE_BASE + 0x0000300)
#define FB_ADDR (MMIO_BASE + 0x1000000)
#define AUDIO_SBUF_ADDR (MMIO_BASE + 0x1200000)
#endif

extern uint32_t i8042_data_port_base;
extern uint32_t vgactl_port_base, vgactl_port_base_syn;
extern void *vmem;
extern uint32_t screen_size;

void init_vga();
void init_i8042();
void i8042_data_io_handler();

#ifdef SOC_SIM
extern uint8_t flash[FLASH_SIZE];

extern char *mrom_file;
extern uint8_t mrom[MROM_SIZE];
size_t init_mrom(const char *mrom_file);
#endif

#endif