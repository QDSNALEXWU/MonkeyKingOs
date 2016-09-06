#include "IDT.h"

/* Initialize the IDT */
#define KEYBOARD 0x60
#define STAT_REG 0x64
void idt_init()
{
	/* Exception */
	set_structure(0,EXEC,0,KERNEL_CS, &DIVIDE_ERROR_CALL);
	set_structure(1,EXEC,0,KERNEL_CS, &DEBUG);
	set_structure(2,EXEC,0,KERNEL_CS, &NMI);
	set_structure(3,EXEC,0,KERNEL_CS, &INT3);
	set_structure(4,EXEC,0,KERNEL_CS,&OVERFLOW);
	set_structure(5,EXEC,0,KERNEL_CS, &BOUNDS);
	set_structure(6,EXEC,0,KERNEL_CS, &INVALID_OP);
	set_structure(7,EXEC,0,KERNEL_CS,&DEVICE_NOT_AVAILABLE);
	set_structure(8,EXEC,0,KERNEL_CS,&DOUBLE_FAULT);
	set_structure(9,EXEC,0,KERNEL_CS,&COPROCESSOR_SEGMENT_OVERRUN);
	set_structure(10,EXEC,0,KERNEL_CS, &INVALID_TSS);
	set_structure(11,EXEC,0,KERNEL_CS, &SEGMENT_NOT_PRESENT);
	set_structure(12,EXEC,0,KERNEL_CS,&STACK_SEGMENT);
	set_structure(13,EXEC,0,KERNEL_CS,&GENERAL_PROTECTION);
	set_structure(14,EXEC,0,KERNEL_CS, &PAGE_FAULT_CALL);
	set_structure(16,EXEC,0,KERNEL_CS, &COPROCESSOR_ERROR);
	set_structure(17,EXEC,0,KERNEL_CS,&ALIGNMENT_CHECK);

	/* Interrupt */
	
	set_structure(0x21,INTR,0,KERNEL_CS,  &KEYBOARD_CALL);	
	set_structure(0x28,INTR,0,KERNEL_CS,  &RTC_CALL);	

	set_structure(0x80,SYS,3,KERNEL_CS,  &SYS_CALL);
	
}

/* Set structure function 
 * input:
 * 	 index:index in the IDT table to be set
 *	 gate_type: the type of the IDT entry
 *	 dpl: descriptor priviledge level 
 * 	 segment: determine segment 
 * 	 handler:pointer to the exception or interrupt 
 * output:none
 */
void set_structure(unsigned int index, int gate_type, unsigned dpl, unsigned segment,void* handler)
{
	//get the old strc
	idt_desc_t str = idt[index];

	// Set point to the structure 
	SET_IDT_ENTRY(str, handler);
	//intialize the elements
	str.seg_selector = (uint16_t)segment;	
	str.present = 1;	
	str.dpl = (uint32_t)dpl;				
	str.reserved4 = 0;						
	str.size = 1;					
	str.reserved1 = 1;
	str.reserved2 = 1;
	switch(gate_type)
	{
		// Reserved bits for EXCEPTIONS
		case EXEC:
			str.reserved0 = 0;
			str.reserved3 = 1;
			break;

		//SYS_GATE has the same setup with INTR_GATE
		case SYS:
		case INTR:
			str.reserved0 = 0;		
			str.reserved3 = 0;
			break;
	}

	//put the new one back
	idt[index] = str;
}


//EXEC HANDLERS
void DIVDE_ERROR(){
		cli();
		//print to terminal
		printf("Dividing by zero error!\n");
	    while(1);
}
void DEBUG(){}
void NMI(){}
void INT3(){}
void OVERFLOW(){}
void BOUNDS(){}
void INVALID_OP(){}
void DEVICE_NOT_AVAILABLE(){}
void DOUBLE_FAULT(){}
void COPROCESSOR_SEGMENT_OVERRUN(){}
void INVALID_TSS(){}
void SEGMENT_NOT_PRESENT(){}
void STACK_SEGMENT(){}
void GENERAL_PROTECTION(uint16_t error){
	cli();
	printf("!!!!!!!!!GENERAL_PROTECTION!!!!!!!!!\n");
	printf("Error code is %x\n", error);
	while(1);
}
void PAGE_FAULT(int32_t error,uint32_t error_loc){
	cli();
	printf("!!!!!!!!!PAGE FAULT!!!!!!!!!\n");
	printf("Page fault! Error code is %x at location %#x\n", error,error_loc);
	while(1);
}


void COPROCESSOR_ERROR(){}
void ALIGNMENT_CHECK(){}

/* 
 * rtc_handler
 *	 inputs: none			   			 
 *   outputs: none
 *   side effect: none
 */
void RTC_HANDLER(){
	//clear register c
	outb(REGISTER_C, RTC_INDEX_PORT);
	inb(RTC_DATA_PORT);
	// call test 
	//test_interrupts();

	//rtc test
	//printf("RTC");
	rtc_flag=0;
	send_eoi(rtc_numer);
}

void SYSCALL_HANDLER(){
	printf("Got a sys call!\n");
}
