/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask=0xff; /* IRQs 0-7 */
uint8_t slave_mask=0xff; /* IRQs 8-15 */
uint16_t catched_irq=0x0000;

/* Initialize the 8259 PIC */
void
i8259_init(void)
{
	uint32_t flags;
	cli_and_save(flags);
	//mask all interrupts
	outb(master_mask,MASTER_8259_IMR);
	outb(slave_mask,SLAVE_8259_IMR);
	//initialize master
	outb(ICW1,MASTER_8259_PORT);
	outb(ICW2_MASTER,MASTER_8259_IMR);
	outb(ICW3_MASTER,MASTER_8259_IMR);
	outb(ICW4,MASTER_8259_IMR);
	//initialize slave
	outb(ICW1,SLAVE_8259_PORT);
	outb(ICW2_SLAVE,SLAVE_8259_IMR);
	outb(ICW3_SLAVE,SLAVE_8259_IMR);
	outb(ICW4,SLAVE_8259_IMR);
	restore_flags(flags);
	return;
}

/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
	uint32_t flags;
	cli_and_save(flags);
	uint8_t irq_mask;
	//if unmask in slave
	if (irq_num&8){
		irq_mask=~(1<<(irq_num-8));
		slave_mask&=irq_mask;
		outb(slave_mask,SLAVE_8259_IMR);
	}
	//if unmask in master
	else{
		irq_mask=~(1<<irq_num);
		master_mask&=irq_mask;
		outb(master_mask,MASTER_8259_IMR);
	}
	restore_flags(flags);
	return;
}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
	uint32_t flags;
	cli_and_save(flags);
	uint8_t irq_mask;
	//if mask in slave
	//check whether is bigger than 8
	if (irq_num&8){
		irq_mask=1<<(irq_num-8);
		slave_mask|=irq_mask;
		outb(slave_mask,SLAVE_8259_IMR);
	}
	//if mask in master
	else{
		irq_mask=1<<irq_num;
		master_mask|=irq_mask;
		outb(master_mask,MASTER_8259_IMR);
	}
	restore_flags(flags);
	return;
}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
	uint32_t flags;
	cli_and_save(flags);
	//if interrupt is from slave
	//check whether is bigger than 8
	if (irq_num&8){
		inb(SLAVE_8259_IMR);
		outb(slave_mask,SLAVE_8259_IMR);
		outb(EOI|(irq_num&7),SLAVE_8259_PORT);
		outb(EOI|PIC_CASCADE_IR,MASTER_8259_PORT);
	}
	//if interrupt is from master
	else{
		inb(MASTER_8259_IMR);
		outb(master_mask,MASTER_8259_IMR);
		outb(EOI|irq_num,MASTER_8259_PORT);
	}
	restore_flags(flags);
	return;
}

