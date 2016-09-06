#ifndef TERMINAL_H
#define TERMINAL_H
#define limit 128
#define process 4

int32_t read_terminal(int32_t fd,char* buf, int32_t nbytes);
int32_t write_terminal(int32_t fd,char* buf, int32_t nbytes);
int32_t open_terminal(const uint8_t* filename);
int32_t close_terminal(int32_t fd);

void KEYBOARD_INIT();
void KEYBOARD_HANDLER();
//need to add to the init function 
void set_cursor (int x , int y ) ;

char line_buffer[limit] ;
int mark  ;
int mark_save;
int cap_flag  ; 
int left_shift_flag  ; 
int right_shift_flag ;
int ctrl_flag  ; 
int l_flag;
int alt_flag ; 
int blank_flag ;
int row ; 
int col ;
int two_line_flag;
int enter_flag[process];


#endif	// TERMINAL_H
