#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H


#include "paging.h"
#include "fileSystem.h"
#include "rtc.h"
#include "x86_desc.h"
#include "terminal.h"
#include "lib.h"

#define NUM_128MB 0x08000000
#define PROG_OFFSET 0x00048000
#define FLAG_MASK 0x00000200
#define KERNEL_STACK_BOTTOM 0x00800000
#define NUM_8KB 8192
#define USER_STACK 0x08400000
#define USER_MASK 0x03
#define NEXT_AVAILABLE_PDE 33
#define VIDMAP_PAGE_MASK 0x07
#define ARG_MAX 128



int32_t cur_pid;
int32_t halt (uint8_t status);

int32_t execute(const uint8_t* command);

int32_t read(int32_t fd, void*buf, int32_t nbytes);

int32_t write(int32_t fd, const void* buf, int32_t nbytes);

int32_t open(const uint8_t* filename);

int32_t close(int32_t fd);

int32_t getargs(uint8_t*buf, int32_t nbytes);

int32_t vidmap(uint8_t** screen_start);

int32_t set_handler(int32_t signum, void* handler_address);

int32_t sigreturn (void);
struct PCB_struct* get_pcb(int32_t index);
int32_t halt(uint8_t status);
int32_t parse_exe_name_arg(uint8_t* command,uint8_t* buffer,uint8_t* args);
int32_t getargs(uint8_t * buf, int32_t nbytes);

#endif

