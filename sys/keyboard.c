#include <sys/sbunix.h>
#include <sys/utility.h>

//scan codes for shift key
#define LEFTSHIFTDOWN 0x2a
#define RIGHTSHIFTDOWN 0x36
#define LEFTSHIFTUP 0xaa
#define RIGHTSHIFTUP 0xb6
#define CONTROLDOWN 0x1d
#define CONTROLUP 0x9d

int shift_flag = 0;
int control_flag = 0;

/*TODO: print the last pressed glyph next to time*/
		

void keyboard_handler(){

unsigned char kdbus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',     /* Tab */
  'q', 'w', 'e', 'r', /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,      /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '`',   0,    /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',      /* 49 */
  'm', ',', '.', '/',   0,        /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

unsigned char shift_kdbus[128] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', '\b', /* Backspace */
  '\t',     /* Tab */
  'Q', 'W', 'E', 'R', /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
    0,      /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
 '\"', '~',   0,    /* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',     /* 49 */
  'M', '<', '>', '?',   0,        /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

	unsigned char scancode;

    /* Read from the keyboard's data buffer */
    scancode = inportb(0x60);
    char carat = '^'; 

    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80)
    {
    	//check if shift/ctrl key has been released
        if((scancode == RIGHTSHIFTUP) || (scancode == LEFTSHIFTUP))
        	shift_flag = 0;
        if((scancode == CONTROLUP))
        	control_flag = 0;
    }
    else
    {
        /* Here, a key was just pressed. Please note that if you
        *  hold a key down, you will get repeated key press
        *  interrupts. */

        //shift/ctrl key is pressed, setting the shift_flag
        if((scancode == RIGHTSHIFTDOWN) || (scancode == LEFTSHIFTDOWN))
           	shift_flag = 1;
        else if((scancode == CONTROLDOWN))
           	control_flag = 1;

        //print the corresponding shift character
        //ignore the printing of shift characters
        if((shift_flag == 1) && !((scancode == RIGHTSHIFTDOWN) || (scancode == LEFTSHIFTDOWN)) )
        {
        	//printf("%x\n", scancode);
            print_char(shift_kdbus[scancode]);
        }
        else if((control_flag == 1) && (scancode != CONTROLDOWN) )
        {
        	//printf("%x\n", scancode);
            print_char(carat);
            print_char(kdbus[scancode]);
        }
        //shift key is not pressed, print the normal characters
        else if(shift_flag == 0){
        	//printf("%x\n", scancode);
        	print_char(kdbus[scancode]);
        }
        	
        
    }
}