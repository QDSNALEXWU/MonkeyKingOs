#ifndef RTC_H
#define RTC_H


#include "lib.h"
#include "i8259.h"
#include "types.h"


#define REGISTER_A 0x0A
#define REGISTER_B 0X0B
#define REGISTER_C 0X0C
#define PERIOD_BIT		0x40
#define RTC_DATA_PORT		0x71
#define RTC_INDEX_PORT	0x70
#define OLD_REG_A_MASK	0xf0

extern uint8_t rtc_flag;

int32_t rtc_open(const uint8_t* filename);

int32_t rtc_write_sys(int32_t fd, const void * buf, int32_t nbytes);

int32_t rtc_read_sys(int32_t fd, const void * buf, int32_t nbytes);
int32_t rtc_write(uint32_t freq);

int32_t rtc_close(int32_t fd);

int32_t rtc_read();

#endif
