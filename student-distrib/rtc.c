#include "rtc.h"

uint8_t rtc_flag=0;
/* rtc_open
 * input:none
 * output:none
 * side effect: initialize the rtc and set the frequency at 2HZ
 */
int32_t rtc_open(const uint8_t* filename){
	int init=rtc_write(2);
	// 4 means success
	if (init!=4){
		printf("RTC init error!!\n");
		return -1;
	}

	//use RegisterB
	outb(REGISTER_B, RTC_INDEX_PORT);

	// Read value from registerB 
	char current = inb(RTC_DATA_PORT);

	//Begin write to Register B
	outb(REGISTER_B, RTC_INDEX_PORT);

	//Turn on bit 6 of register B so that periodic refresh can be achiceved
	outb(PERIOD_BIT	 | current, RTC_DATA_PORT);

	//clear register
	outb(REGISTER_C, RTC_INDEX_PORT);
	inb(RTC_DATA_PORT);

	return 0;
}
/* rtc_write
 * input:rtc frequency
 * output:interger to indicate whether success or not
 * side effect: change the rtc frequency
 */

int32_t rtc_write_sys(int32_t fd, const void * buf, int32_t nbytes){
	rtc_write(*(uint32_t*)buf);
	return 0;
}
int32_t rtc_write(uint32_t freq){
	uint8_t mask;
	switch (freq){
		//different frequency will lead to different setting of the register bits
		case 2: mask=0x0f;
				break;
		case 4: mask=0x0e;
				break;
		case 8: mask=0x0d;
				break;
		case 16: mask=0x0c;
				break;
		case 32: mask=0x0b;
				break;
		case 64: mask=0x0a;
				break;
		case 128: mask=0x09;
				break;
		case 256: mask=0x08;
				break;
		case 512: mask=0x07;
				break;
		case 1024: mask=0x06;
				break;
		default: printf("ERROR!!!!!");
				return -1;
	}
	//set frequency 
	outb(REGISTER_A,RTC_INDEX_PORT);
	char old_data=inb(RTC_DATA_PORT);
	outb(REGISTER_A,RTC_INDEX_PORT);
	outb((old_data&OLD_REG_A_MASK)|mask,RTC_DATA_PORT);
	return 4;
}
/* rtc_close
 * input:
 * output:interger to indicate whether success or not
 * side effect: none
 */
int32_t rtc_close(int32_t fd){
	return 0;
}
/* rtc_read
 * input:none
 * output:interger to indicate whether success or not
 * side effect: wait until next interrupt to turn on the rtc again
 */
 int32_t rtc_read_sys(int32_t fd, const void * buf, int32_t nbytes){
 	rtc_read();
 	return 0;
 }
int32_t rtc_read(){
	sti();
	rtc_flag=1;
	//wait until next interrupt
	while (rtc_flag);
	return 0;
}
