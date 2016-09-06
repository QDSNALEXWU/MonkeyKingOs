#include "lib.h"
#include "terminal.h"
#include "i8259.h"

//!!!!!!!!Assume the port of keyboard is 0x60, can be changed here !!!!!!!!!!
//http://flint.cs.yale.edu/cs422/doc/art-of-asm/pdf/APNDXC.PDF
//or google keyboard scancode 

#define STAT_REG 0x64
#define KEYBOARD 0x60
#define keyboard_number 1 
#define limit 128
#define LINE_MAX 80
#define MAX_SCAN 58
#define y_max 25
int enter_flag[4]={0};
int mark_offset = 7;

// characters array
unsigned char display_a [] = { 

'@'  , '!' , '@' , '#' , '$' , 

'%' , '^' , '&' , '*' , '(' , 

')' , '_' , '+' , '@' , '@' , 

'Q' , 'W' , 'E' , 'R' , 'T' , 

'Y' , 'U' , 'I' , 'O' , 'P' , 

'{' , '}' , '@' , '@' , 'A' , 

'S' , 'D' , 'F' , 'G' , 'H' , 

'J' , 'K' , 'L' , ':' , '"' , 

'~' , '@' , '|' , 'Z' , 'X' , 

'C' , 'V' , 'B' , 'N' , 'M' , 

'<' , '>' , '?' 
};


unsigned char display_b [] ={
'@' , '1' , '2' , '3' , '4' ,

'5' , '6' , '7' , '8' , '9' ,

'0' , '-' , '=' , '@' , '@' ,

'q' , 'w' , 'e' , 'r' , 't' ,

'y' , 'u' , 'i' , 'o' , 'p' ,

'[' , ']' , '@' , '@' , 'a' ,

's' , 'd' , 'f' , 'g' , 'h' ,

'j' , 'k' , 'l' , ';' , '\'' ,

'`' , '@' , '\\' , 'z' , 'x' ,

'c' , 'v' , 'b' , 'n' , 'm' ,

','  , '.' , '/'   
};

unsigned char display_c [] ={
'@' , '1' , '2' , '3' , '4' ,

'5' , '6' , '7' , '8' , '9' ,

'0' , '-' , '=' , '@' , '@' ,

'Q' , 'W' , 'E' , 'R' , 'T' ,

'Y' , 'U' , 'I' , 'O' , 'P' ,

'[' , ']' , '@' , '@' , 'A' ,

'S' , 'D' , 'F' , 'G' , 'H' ,

'J' , 'K' , 'L' , ';' , '\'' ,

'`' , '@' , '\\' , 'Z' , 'X' ,

'C' , 'V' , 'B' , 'N' , 'M' ,

','  , '.' , '/'   
};


/*KEYBOARD_INIT
*input: None
*output: None
*side effect: initial keyboard */
void KEYBOARD_INIT(){
    //set all flags
    mark = 0  ;
    mark_save=0;
    cap_flag = 0  ; 
    left_shift_flag = 0 ; 
    right_shift_flag = 0 ; 
    ctrl_flag = 0  ; 
    alt_flag = 0 ; 
    row = 0 ; 
    col = 0 ;
    blank_flag = 0 ;
    l_flag=0;
    two_line_flag=0;
    //clear screen
    clear();

}

/*scancode_to_char
*input: input keypress
*output: accordingly character printed out on the screen
*side effect: print out character value has to - 1 */
unsigned char 
scancode_to_char (unsigned char input  ){

   int index = (int)input ;  
   index -- ; 
   //return the correct character depending on flags
   if( left_shift_flag || right_shift_flag )
   return display_a[index] ;
   else if (cap_flag)
    return display_c[index];
   else 
   return display_b[index] ;
}

/*set_cursor
*input:x,y coordinates
*output: none
*side effect: change the cursor location */
void 
set_cursor (int x , int y ) {

    int p = (y * NUM_COLS) + x;
    //set cursor 
    outb(0x0F, 0x3D4);
    outb((unsigned char)(p&0xFF), 0x3D5);

    outb(0x0E, 0x3D4);
    outb((unsigned char )((p>>8)&0xFF), 0x3D5);

} 





/*KEYBOARD_HANDLER
 *INPUT:NONE
 *OUTPUT:NONE
 *SIDE EFFECT: Print out the keypress*/
void KEYBOARD_HANDLER(){

    // Clear status register 
    inb(STAT_REG);

    unsigned char keyboard_input ; 
    unsigned char keyboard_char ; 

    //get a char at this port 
    keyboard_input = inb(KEYBOARD) ; 
  
    blank_flag = 0 ;


    switch(keyboard_input){
    
    //back space 
    case 0x0E :
    mark = mark - 1  ;
    if (mark<0){
        blank_flag=1;
        mark=0 ;
        break;
    }
    /*if ((two_line_flag==1)&&(mark<=78)){
        two_line_flag=0;
    }*/
    col  = col - 1 ;
    line_buffer[mark] = ' ' ; 
    screen_y = row ;
    screen_x = col  ;
    printf(" ");
    screen_x-=1;
    if (col<0){
        col=NUM_COLS-1;
        row-=1;
    }
    screen_y=row;
    screen_x=col;
    set_cursor(screen_x,screen_y);
    blank_flag = 1 ;
    break ;
 
    case 0x8E:
    //tab 
    case 0x0F:
    case 0x8F:
    blank_flag = 1 ;

    break ;

    //enter 
    case 0x1C :
    mark_save=mark;
    mark = 0 ; 
    row = row + 1 ;
    if (row == NUM_ROWS ) {
    vertical_scrolling();
    row-=1; 
     screen_x=0;
     screen_y=NUM_ROWS-2;
    } 
    col=0;
    screen_y = screen_y + 1 ;
    screen_x=0;
    set_cursor(screen_x,screen_y) ;
    blank_flag = 1 ;
    two_line_flag=0;
    enter_flag[0]=1;
    break ;

    case 0x9C : 
    blank_flag = 1 ;
    break ;


    //ctrl + l 
    case 0x26 :
    l_flag=1;
    break ;

    case 0xa6:
    l_flag=0;
    break;





 
    //left shift 
    case 0x2A :
    left_shift_flag = 1 ; 
    blank_flag = 1 ;
    break ;
    
    case 0xAA :
    left_shift_flag = 0 ; 
    blank_flag = 1 ;
    break ;
    
    //right shift 
    case 0x36 : 
    right_shift_flag = 1 ;
    blank_flag = 1 ;
    break ;
    
    case 0xB6 :
    right_shift_flag = 0 ; 
    blank_flag = 1 ;
    break ;
    
    //alt 
    case 0x38: 
    alt_flag = 1 ;
    blank_flag = 1 ;
    break ;
    
    case 0xb8:
    alt_flag = 0 ;
    blank_flag = 1 ;
    break ;


    //CAP 
    case 0x3A:
    cap_flag = !cap_flag ;
    blank_flag = 1 ;
    break ;

    case 0xB9: 
    blank_flag = 1 ;
    break ;

    
    //ctrl
    case 0x1D:
    ctrl_flag =1 ; 
    blank_flag = 1 ;
    break;

    case 0x9D:
    ctrl_flag = 0 ; 
    blank_flag = 1 ;
    break;

   

    default:
    break ;

    } 

    //control+l clears screen
    if (l_flag&&ctrl_flag){
      
        clear() ;
        two_line_flag=0;
        set_cursor(0,0) ; 
        col=0 ;
        row=0 ; 
        mark=0 ; 
        mark_save=0;
        send_eoi(keyboard_number) ;
        return;
    }
   

    //control+k write to terminal
     if (alt_flag&&ctrl_flag){
        row++;
        if (row == NUM_ROWS ) {
            vertical_scrolling();
            row-=1; 
            screen_x=0;
            screen_y=NUM_ROWS-2;
        } 
        printf("\n");
        write_terminal(0,line_buffer,mark) ;  
        send_eoi(keyboard_number) ;
        return;
   }

  


    //58 is the highest scancode we will use 
    if(keyboard_input > 0 && keyboard_input < MAX_SCAN && (!blank_flag) ){
    
    if(mark==limit + mark_offset){
    //assume irq is 1 
    send_eoi(keyboard_number) ;
    return ;
    }
    //if we reach the end of one line
    if(mark + mark_offset == NUM_COLS){
        screen_y=screen_y+1 ; 
        row=row+1; 
        two_line_flag=1;
        col=0 ; 
        //if we reaches the last row
        if (row == NUM_ROWS ) {
            vertical_scrolling(); 
            row-=1;
            screen_x=0;
            screen_y=NUM_ROWS-1;
        } 
        set_cursor(col,row) ;
    }
    //output and store the key
    keyboard_char = scancode_to_char(keyboard_input);
    printf("%c", keyboard_char );
    line_buffer[mark] = keyboard_char ;
    mark++ ;
    col ++ ; 
    if(col<=NUM_COLS)
    set_cursor(col,row) ;
       
    }

    //assume irq is 1 
    send_eoi(keyboard_number) ;
}
/* read_terminal
 * input:terminal buffer, number of bytes to read
 * output:none
 * side effect: save the input into a buffer
 */
int32_t read_terminal(int32_t fd,char* buf, int32_t nbytes){
    cli();
    while(1){
        if (enter_flag[0]==1){
            memcpy(buf,line_buffer,nbytes);
            int i;
            for (i=mark_save;i<nbytes;i++){
                buf[i]='\0';
            }
           /* for(i =0 ; i< mark_save;i++){
              //printf("%c",buf[i]);  
            }*/
            
            //printf("%d, %d\n",mark_save,mark);
            enter_flag[0]=0;
            return mark_save;
        }
        sti();
    }

}
/* write_terminal
 * input:terminal buffer, number of bytes to read
 * output:none
 * side effect: output the buffer contect to the screen
 */
//when enter is pressed, store all the info
int32_t write_terminal(int32_t fd,char* buf, int32_t nbytes){

    int i ;
    //write all chars in buffer
    if(nbytes>limit + mark_offset )
    nbytes = limit + mark_offset ;    

    for(i=0;i<nbytes;i++){
        printf("%c",buf[i]);
        //line_buffer[i] = buf[i] ;
        if(i==LINE_MAX){
            screen_y++ ; 
            
            if(screen_y == y_max ){
            vertical_scrolling(); 
            screen_y=y_max -1 ;
            }
             


            screen_x=0 ;
        } 
    }
    
    
    //set cursor
    col=screen_x ; 
    row=screen_y;
    set_cursor(screen_x,screen_y); 
    return nbytes;
    //memcpy(line_buffer,buf,mark) ;
 

}
/* open_terminal
 * input:filename
 * output:none
 * side effect: turn on the keyboard
 */

int32_t open_terminal(const uint8_t* filename){

    KEYBOARD_INIT(); 
    return 0;
}
/* close_terminal
 * input:filename
 * output:none
 * side effect: 
 */
int32_t close_terminal(int32_t fd){
    return 0;
}

