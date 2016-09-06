#ifndef FILESYSTEM_H   //if not defined, define it.
#define FILESYSTEM_H


#include "types.h"
#include "multiboot.h"
#include "lib.h"
#include "terminal.h"
#include "system_call.h"


typedef uint32_t (*open_function)(const uint8_t*);
typedef uint32_t (*read_function)(int32_t, void*, int32_t);
typedef uint32_t (*write_function)(int32_t, const void*, int32_t);
typedef uint32_t (*close_function)(int32_t);
// the struct of a directory entry
typedef struct dir_entry{
	char file_name[32];   // 32B
	int32_t file_type;   // 0: user_level 1: directory 2: regular file
	int32_t inode;   //only meaningful for regular file
}dir_entry;

typedef struct file_descriptor
{
	struct file_op_jmp_table*  file_table;
	uint32_t inode;
	uint32_t position;
	uint32_t flags;
	
}file_descriptor;



typedef struct file_op_jmp_table{
	
	  open_function file_op_open;
	  read_function file_op_read;
	  write_function file_op_write;
	  close_function file_op_close;

	  	  

}file_op_jmp_table;


typedef struct file_descriptor_array{

	file_descriptor files[8];

}file_descriptor_array;

typedef struct PCB_struct{
	uint32_t pcb_index;
	struct PCB_struct *pre_PCB;  // parent program pcb
    file_descriptor_array fd_array;
	//uint32_t 	signal;
	uint32_t 	return_adrs;
	uint32_t	ebp;
	uint32_t	esp0;//kernel
	uint32_t	esp;
	uint32_t    cr3;
	uint32_t	ss;
	uint32_t 	old_cr3;
	uint32_t *pg_dir;
	uint8_t *args;
}PCB_struct;

file_op_jmp_table std_in_jmp_table;
file_op_jmp_table std_out_jmp_table;
file_op_jmp_table file_jmp_table;
file_op_jmp_table rtc_jmp_table;
file_op_jmp_table dir_jmp_table;




uint32_t FILE_SYS_START;


int32_t write_file_system( int32_t fd, const void* buf , int32_t nbytes) ;
int32_t open_file_system( const uint8_t* filename) ;
int32_t close_file_system(int32_t fd);



int32_t read_dentry_by_name (const uint8_t* fname, dir_entry* dentry);
int32_t read_dentry_by_index (uint32_t index, dir_entry* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t get_file_length(dir_entry dentry);
void get_fs_start(uint32_t start);
int32_t ls_directory();
int32_t read_file_sys(int32_t fd , void* buf, uint32_t nbytes);
int32_t read_file_test(uint8_t* fname,int32_t length);
int32_t read_directory(int32_t fd, void* buf, int32_t nbytes);
int32_t open_directory(const uint8_t * filename);
int32_t write_directory( int32_t fd, const void* buf , int32_t nbytes);
int32_t close_directory(int32_t fd);
int32_t check_executable(dir_entry dentry);
uint32_t get_entry_point(dir_entry dentry);

void fd_array_init(file_descriptor_array* array);



#endif
