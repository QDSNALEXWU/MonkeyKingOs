#include "system_call.h"



#define RTC 0
#define DIR 1
#define FILE  2
#define fd_min   0
#define fd_max   7
#define FOUR_KB 4096
#define max_inode 999
#define max_input 128

int32_t cur_pid=-1;

uint32_t __attribute__((aligned (TABLE_ALLIGN))) vidmap_table[1];
/* 
 * execute 
 *   Description: exectue process on command, it will create new page for the new process and save the PCB of the old process
 *	 INPUTS: pointer to command		 
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failfure
 *   SIDE EFFECTS: change the paging setting
 */
int32_t execute(const uint8_t* command){
	//check for command
	if (!command){
		printf("Error: command is NULL!\n");
		return -1;
	}
	//get name of executable
	uint8_t exe_name[max_input]={0};
	uint8_t argument[max_input]={0};
	/*for (i=0;i<128;i++){
		printf("%x ",command[i]);
	}*/
	//printf("\n");
	if(parse_exe_name_arg((uint8_t*)command,exe_name,argument)==-1){
		printf("Error: invalid command!\n");
		return -1;
	}
	
	//printf("%s\n",exe_name);
	//printf("%x\n",argument[i]);
	
	//find inode of executable
	dir_entry d_entry;
	int32_t dentry=read_dentry_by_name(exe_name,&d_entry);
	if (dentry==-1){
		printf("Error: invalid dentry!\n");
		return -1;
	}
	//check if the file is a executable
	if (check_executable(d_entry)!=0){
		printf("Error: file is not an executable!\n");
		return -1;
	}
	//allocate new process
	int32_t p_index=find_avail_process();
	if (p_index==-1){
		printf("Error: max number of process reached!\n");
		return -1;
	}
	//save old process's esp,ss,cr3    // cli ? sti? 
	uint32_t old_cr3,old_esp,old_ss,old_ebp;
	cli();
	asm volatile("movl %%cr3,%0\n\t": "=r"(old_cr3):);
	asm volatile("movl %%esp,%0\n\t": "=r"(old_esp):);  // poential bug?
	asm volatile("movl %%ss,%0\n\t": "=r"(old_ss):);
	asm volatile("movl %%ebp,%0\n\t": "=r"(old_ebp):);
	sti();

	uint32_t new_cr3=allocate_new_page(p_index);
	//reload cr3 to switch to new page table
	asm volatile("movl %0, %%cr3\n\t": : "r"(new_cr3));
	//get entry point
	uint32_t entry_point=get_entry_point(d_entry);
	//parse the entire program
	int32_t length=get_file_length(d_entry);
	//printf("%x\n",length);
	read_data(d_entry.inode,0,(uint8_t*)(NUM_128MB+PROG_OFFSET),length);
	//change esp0 and ss0
	//pcb 

	//printf("=========checkpoint1=========\n");
	PCB_struct New_PCB;
	New_PCB.pcb_index = p_index;  // +1???
	New_PCB.ss = old_ss;
	New_PCB.esp = old_esp;
	New_PCB.ebp = old_ebp;
	New_PCB.esp0 = tss.esp0;
	New_PCB.old_cr3 = old_cr3;
	New_PCB.args=argument;
	New_PCB.cr3=new_cr3;
	//printf("=========checkpoint2=========\n");
	//New_PCB.pg_dir =
	if (cur_pid==-1)
		New_PCB.pre_PCB=NULL;
	else
		New_PCB.pre_PCB=get_pcb(cur_pid);
	cur_pid=p_index;
	fd_array_init(&(New_PCB.fd_array));
	//printf("=========checkpoint3=========\n");

	memcpy((void*)get_pcb(p_index),(const void*)(&New_PCB),sizeof(PCB_struct));

	//printf("=========checkpoint4=========\n");
	tss.ss0=KERNEL_DS;
	tss.esp0=KERNEL_STACK_BOTTOM-p_index*NUM_8KB-NUM_8KB-4;

	cli();
	//http://www.jamesmolloy.co.uk/tutorial_html/10.-User%20Mode.html
	asm volatile("movw %0, %%ax\n\t": :"g"(USER_DS));
	asm volatile("movw %%ax, %%ds\n\t": :);
	asm volatile("pushl %0\n\t": :"g"(USER_DS));
	asm volatile("pushl %0\n\t": :"g"(USER_STACK-4));
	//modify eflags
	asm volatile("pushfl\n\t": :);
	asm volatile("popl %%eax\n\t": :);
	asm volatile("orl $0x200,%%eax\n\t": :);
	asm volatile("pushl %%eax\n\t": :);
	//push for iret
	asm volatile("pushl %0\n\t": :"g"(USER_CS));
	asm volatile("pushl %0\n\t": :"g"(entry_point));
	asm volatile("iret_label:\n\t"::);
	asm volatile ("iret\n\t": :"g"(entry_point)); 
	asm volatile("halt_return_label:\n\t"::);
	//printf("=========checkpoint10=========\n");
	uint32_t ret_val;
	asm volatile("movl %%ebx,%0": "=g"(ret_val):);
	sti();
	return ret_val;
}
/* 
 * parse_exe_name_arg
 *   Description: save the command and argument into different buffers
 *	 INPUTS: pointer to command, pointer to comand buffer, pointer to argument buffers	   			 
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failfure
 *   SIDE EFFECTS: none
 */
int32_t parse_exe_name_arg(uint8_t* command,uint8_t* buffer,uint8_t* args){
	if (!command)
		return -1;
	int32_t i=0;
	//parse exe name
	while (command[i]!=' '&&command[i]!='\0'){
		buffer[i]=command[i];
		i++;
	}
	buffer[i]='\0';
	//parse arguments
	i++;
	while (command[i]!='\0'&&i<max_input){
		*args=command[i];
		args++;
		i++;
	}
	*args='\0';
	
	return 0;
}


/* 
 * read
 *   Description: Read the file according to the file type
 *	 INPUTS: file_descriptor, buffer to store, number of bytes to read	   			 
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failfure
 *   SIDE EFFECTS: none
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
	if(fd < fd_min){
		return -1;
	}
	if(fd > fd_max){
		return -1;
	}
	//tries to read from stdout
	if (fd==1)
		return -1;
	PCB_struct* pcb;
	pcb=get_pcb(cur_pid);
	//check if fd is opened
	if (pcb->fd_array.files[fd].flags==0)
		return -1;
	return (*pcb->fd_array.files[fd].file_table->file_op_read)(fd, buf, nbytes);
}

/*
 * write
 *   Description: Write to the file
 *	 INPUTS: file descriptor, buffer , number of bytes to write	   			 
 *   OUTPUTS: none
 *   RETURN VALUE: failure(-1) or bytes wrote
 *   SIDE EFFECTS: none
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{

	if(fd < fd_min){
		return -1;
	}
	if(fd > fd_max){
		return -1;
	}
	//tries to write to stdin
	if (fd==0)
		return -1;
	PCB_struct* pcb;
	pcb = get_pcb(cur_pid);
	//check if fd is opened
	if (pcb->fd_array.files[fd].flags==0)
		return -1;

	return (*pcb->fd_array.files[fd].file_table->file_op_write)(fd, buf, nbytes);
}

/* 
 * open
 *   Description: Open a file , and intialize the fd array according to different file type
 *	 INPUTS: pointer to file name
 *   OUTPUTS: none
 *   RETURN VALUE: return -1 when is failure
 *   SIDE EFFECTS: create a file descriptor in the fd array
 */
int32_t open(const uint8_t* filename)
{

	if (filename == NULL) {
		return -1;
	}
	// no filename 
	if(filename[0] == '\0'){
		return -1;
	}

	// if we try to open the stdin and stdout, return the index for them
	if (filename == (uint8_t*)"stdout") {
		return 1;
	}

	if (filename == (uint8_t*)"stdin") {
		return 0;
	}


	dir_entry return_dir;
	//check whether the file is inside the filesystem
	if(read_dentry_by_name(filename, &return_dir) == -1) {
		return -1;
	}

	PCB_struct* pcb;
	
	pcb=get_pcb(cur_pid);


	int return_fd = 0;// initial the return fd array index

	
	int i;

	//look for available fd space
	//2 is the start of empty fd
	for(i = 2; i < fd_max+1; i++) {
		if (pcb->fd_array.files[i].flags == 0) {
			return_fd = i;
			pcb->fd_array.files[i].flags = 1;
			break;
		}
	}


	if(return_fd == 0) {
		return -1;
	}
	//
	switch(return_dir.file_type)
	{
		case RTC:
			rtc_jmp_table.file_op_open = (open_function)&rtc_open;
			rtc_jmp_table.file_op_write = (write_function)&rtc_write_sys;
			rtc_jmp_table.file_op_read = (read_function)&rtc_read_sys;
			rtc_jmp_table.file_op_close = (close_function)&rtc_close;
			pcb->fd_array.files[return_fd].file_table = &rtc_jmp_table;
			pcb->fd_array.files[return_fd].inode = max_inode;
			pcb->fd_array.files[return_fd].position = 0;
			break;

		case DIR:
			dir_jmp_table.file_op_open = (open_function)&open_directory;
			dir_jmp_table.file_op_write = (write_function)&write_directory;
			dir_jmp_table.file_op_read = (read_function)&read_directory;
			dir_jmp_table.file_op_close = (close_function)&close_directory;
			pcb->fd_array.files[return_fd].file_table = &dir_jmp_table;
			pcb->fd_array.files[return_fd].inode = max_inode;
			pcb->fd_array.files[return_fd].position = 0;
			break;

		case FILE:
			file_jmp_table.file_op_open = (open_function)&open_file_system;
			file_jmp_table.file_op_write = (write_function)&write_file_system;
			file_jmp_table.file_op_read = (read_function)&read_file_sys;
			file_jmp_table.file_op_close = (close_function)&close_file_system;
			pcb->fd_array.files[return_fd].file_table = &file_jmp_table;
			pcb->fd_array.files[return_fd].inode = return_dir.inode;
			pcb->fd_array.files[return_fd].position = 0;
			break;
	}

	(*pcb->fd_array.files[return_fd].file_table->file_op_open)(filename);
	
	return return_fd;
}

/* 
 * get_pcb
 *   Description: get the current pcb start address 
 *	 INPUTS: process index
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to the pcb start address
 *   SIDE EFFECTS: none
 */

PCB_struct* get_pcb(int32_t index){
	return (PCB_struct*)(KERNEL_STACK_BOTTOM-index*NUM_8KB-NUM_8KB);
}

/* 
 * close
 *   Description: Close the fd, and clear the fd content
 *	 INPUTS: fd
 *   OUTPUTS: none
 *   RETURN VALUE: -1 with failure
 *   SIDE EFFECTS: none
 */
int32_t close(int32_t fd)
{
	//cant close terminal
	if(fd == 0 || fd == 1) {
		return -1;
	}

	if(fd < 0 || fd > fd_max){
		return -1;
	}
	
	PCB_struct * pcb;
	
	pcb=get_pcb(cur_pid);
     //if the file is not open yet, return failure
	if( pcb->fd_array.files[fd].flags == 0 )  {
		return -1;
	}

	//dereference the function pointer
	if(pcb->fd_array.files[fd].file_table != NULL) {
		(*pcb->fd_array.files[fd].file_table->file_op_close)(fd);
	}

	//reset the fd
	pcb->fd_array.files[fd].inode = max_inode;
	pcb->fd_array.files[fd].file_table = NULL;
	pcb->fd_array.files[fd].position = 0;
	pcb->fd_array.files[fd].flags = 0;

	return 0;
}
/* 
 * halt
 *   Description: halt the process and return control back to the old process by updating the registers
 *	 INPUTS: system status
 *   OUTPUTS: none
 *   RETURN VALUE: -1 with failure,0 on success
 *   SIDE EFFECTS: none
 */
int32_t halt(uint8_t status){

    PCB_struct cur_process;
	cur_process = *(get_pcb(cur_pid));

       //another case 
	if (cur_process.pre_PCB!=NULL)
		cur_pid=cur_process.pre_PCB->pcb_index;
	else
		cur_pid=-1;
    free_process(cur_process.pcb_index) ; 
   // printf("=========checkpoint1=========\n");
    asm volatile ("cli\n\t": :); 
    tss.esp0 = cur_process.esp0;    // I think this should point to kernel_PCB.stack or somewhere else 
    asm volatile ("movl %0, %%cr3\n\t": :"r"(cur_process.old_cr3));  
    asm volatile ("movl %0, %%ebp\n\t": :"g"(cur_process.ebp)); 
    asm volatile ("movl %0, %%esp\n\t": :"g"(cur_process.esp)); 
   //printf("=========checkpoint2=========\n");
    //terminate process 0 
    if(cur_pid == -1)
    {
         asm volatile ("sti\n\t": :);
         printf("Try to quit the shell, get back\n");
		 int i ; 
		 //1000 is a software delay
		 for( i=0 ; i< 1000 ; i++){
		 printf("Shell restart\n");
		 }
         clear();
		 set_cursor(0,0);
		 execute((void*)"shell");


    }
	//printf("=========checkpoint3=========\n");
 	
   //status handling 
   // asm volatile ("movzx %%bl, %%ebx\n\t": :);   

    asm volatile ("sti\n\t": :);
    asm volatile ("jmp halt_return_label\n\t": :);  

      
    printf("I should not be printed out\n");  
    return 0;  

}


int32_t vidmap(uint8_t** screen_start){
	if ((screen_start<(uint8_t**)NUM_128MB)||(screen_start>(uint8_t**)(USER_STACK-4)))
		return -1;
	vidmap_table[0]=VIDEO|VIDMAP_PAGE_MASK;
	uint32_t* page_table=get_pd_ptr(cur_pid);
	page_table[NEXT_AVAILABLE_PDE]=(uint32_t)&vidmap_table[0]|VIDMAP_PAGE_MASK;
	*screen_start=(uint8_t*)USER_STACK;
	return 0;
}

int32_t getargs(uint8_t * buf, int32_t nbytes){
	PCB_struct * pcb;
	pcb = get_pcb(cur_pid);
	int arg_size = strlen((int8_t*)pcb->args);
	int i = 0;

	if(pcb->args == '\0' ){
		return -1;
	}
	if((arg_size+1) > nbytes){
		return -1;
	}
	//initialize the buffer
	for(i = 0; i < nbytes;i++){
		buf[i] = '\0';
	}

	memcpy((void*)buf, (const void*)pcb->args, arg_size+1);
	return 0;
}
