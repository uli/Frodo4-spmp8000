#include <libgame.h>
#include <stdio.h>
#include "sysdeps.h"


#include "spmp_menu_input.h"
extern "C" {
//#include "libpogo/pogo.h"
//#include "ui.h"
//#include "chatboard/kbdrv.h"
}

#include "C64.h"
#include "Prefs.h"

uint32 button_state;
uint32 joy_state;
extern int current_joystick;

//int chatboard_enabled=0;
extern int options_enabled;
extern int keyboard_enabled;
extern int status_enabled;

// button bindings
int start_function=UI;
int start_value=MENU;
int select_function=KEY;
int select_value=1;
int x_function=JOY;
int x_value=FIRE;
int o_function=KEY;
int o_value=0;
int left_function=JOY;
int left_value=LEFT;
int right_function=JOY;
int right_value=RIGHT;
int up_function=JOY;
int up_value=UP;
int down_function=JOY;
int down_value=DOWN;
int lshoulder_function=UI;
int lshoulder_value=VKEYB;
int rshoulder_function=UI;
int rshoulder_value=STATS;

int bkey_pressed;
int bkey_released;
int bkey;

#define MATRIX(a,b) (((a) << 3) | (b))
struct keymapping keybindings[] = {
	{ "space", MATRIX(7,4) },
	{ "shift/run-stop", MATRIX(7,7)|128 },
	{ "c=", MATRIX(7,5) },
	{ "return", MATRIX(0,1) },
	{ "crsr up", MATRIX(0,7)|128 },
	{ "crsr down", MATRIX(0,7) },
	{ "crsr left", MATRIX(0,2)|128 },
	{ "crsr right", MATRIX(0,2) },
	{ "f1", MATRIX(0,4) },
	{ "f2", MATRIX(0,4)|128 },
	{ "f3", MATRIX(0,5) },
	{ "f4", MATRIX(0,5)|128 },
	{ "f5", MATRIX(0,6) },
	{ "f6", MATRIX(0,6)|128 },
	{ "f7", MATRIX(0,3) },
	{ "f8", MATRIX(0,3)|128 },
	{ "0", MATRIX(4,3) },
	{ "1", MATRIX(7,0) },
	{ "2", MATRIX(7,3) },
	{ "3", MATRIX(1,0) },
	{ "4", MATRIX(1,3) },
	{ "5", MATRIX(2,0) },
	{ "6", MATRIX(2,3) },
	{ "7", MATRIX(3,0) },
	{ "8", MATRIX(3,3) },
	{ "9", MATRIX(4,0) },
	{ "a", MATRIX(1,2) },
	{ "b", MATRIX(3,4) },
	{ "c", MATRIX(2,4) },
	{ "d", MATRIX(2,2) },
	{ "e", MATRIX(1,6) },
	{ "f", MATRIX(2,5) },
	{ "g", MATRIX(3,2) },
	{ "h", MATRIX(3,5) },
	{ "i", MATRIX(4,1) },
	{ "j", MATRIX(4,2) },
	{ "k", MATRIX(4,5) },
	{ "l", MATRIX(5,2) },
	{ "m", MATRIX(4,4) },
	{ "n", MATRIX(4,7) },
	{ "o", MATRIX(4,6) },
	{ "p", MATRIX(5,1) },
	{ "q", MATRIX(7,6) },
	{ "r", MATRIX(2,1) },
	{ "s", MATRIX(1,5) },
	{ "t", MATRIX(2,6) },
	{ "u", MATRIX(3,6) },
	{ "v", MATRIX(3,7) },
	{ "w", MATRIX(1,1) },
	{ "x", MATRIX(2,7) },
	{ "y", MATRIX(3,1) },
	{ "z", MATRIX(1,4) },
	{ NULL, NULL }
};

extern void reboot_gp32(void);

void process_input(int pressed, char function, char value) {
	if(function==JOY) {
		switch(value) {
			case FIRE:
				if(pressed) {
					joy_state|=FIRE_PRESSED;
				} else {
					joy_state&=~(uint32)FIRE_PRESSED;
				}
				return;
			case LEFT:
				if(pressed) {
					joy_state|=LEFT_PRESSED;
				} else {
					joy_state&=~(uint32)LEFT_PRESSED;
				}
				return;
			case RIGHT:
				if(pressed) {
					joy_state|=RIGHT_PRESSED;
				} else {
					joy_state&=~(uint32)RIGHT_PRESSED;
				}
				return;
			case UP:
				if(pressed) {
					joy_state|=UP_PRESSED;
				} else {
					joy_state&=~(uint32)UP_PRESSED;
				}
				return;
			case DOWN:
				if(pressed) {
					joy_state|=DOWN_PRESSED;
				} else {
					joy_state&=~(uint32)DOWN_PRESSED;
				}
				return;
		}
	} else if(function==UI) {
		switch(value) {
			case MENU:
				if(pressed) options_enabled=~options_enabled;
				return;
			case VKEYB:
				if(pressed) keyboard_enabled=~keyboard_enabled;
				return;
			case STATS:
				if(pressed) status_enabled=~status_enabled;
				return;
		}
	} else if(function==KEY) {
		bkey=keybindings[value].key;
		if(pressed) {
			bkey_pressed=1;
		} else {
			bkey_released=1;
		}
	}
}

extern uint32_t keys;
extern emu_keymap_t keymap;

void poll_input(void) {
	static uint32 last_pbdat=0;
	uint32 current_pbdat;

	keys = emuIfKeyGetInput(&keymap);
	current_pbdat = keys ^ 0xffffffffUL;
	if(!last_pbdat) last_pbdat= current_pbdat;

	if(current_pbdat!=last_pbdat) {
		/* left */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_LEFT]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_LEFT]) {
				button_state&=~(uint32)LEFT_PRESSED;
				process_input(0, left_function, left_value);
			} else {
				button_state|=LEFT_PRESSED;
				process_input(1, left_function, left_value);
			}
		}
		/* right */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_RIGHT]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_RIGHT]) {
				button_state&=~(uint32)RIGHT_PRESSED;
				process_input(0, right_function, right_value);
			} else {
				button_state|=RIGHT_PRESSED;
				process_input(1, right_function, right_value);
			}
		}
		/* up */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_UP]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_UP]) {
				button_state&=~(uint32)UP_PRESSED;
				process_input(0, up_function, up_value);
			} else {
				button_state|=UP_PRESSED;
				process_input(1, up_function, up_value);
			}
		}
		/* down */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_DOWN]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_DOWN]) {
				button_state&=~(uint32)DOWN_PRESSED;
				process_input(0, down_function, down_value);
			} else {
				button_state|=DOWN_PRESSED;
				process_input(1, down_function, down_value);
			}
		}
		/* left shoulder button */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_L]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_L]) {
				button_state&=~(uint32)LSHOULDER_PRESSED;
				process_input(0, lshoulder_function, lshoulder_value);
			} else {
				button_state|=LSHOULDER_PRESSED;
				//if(!keyboard_enabled) keyboard_enabled=1;
				//else keyboard_enabled=0;
				process_input(1, lshoulder_function, lshoulder_value);
			}
		}
		/* button B */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_O]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_O]) {
				button_state&=~(uint32)O_PRESSED;
				process_input(0, o_function, o_value);
			} else {
				button_state|=O_PRESSED;
				process_input(1, o_function, o_value);
			}
		}
		/* button A */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_X]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_X]) {
				//printf("X released %0x/%0x\n", current_pbdat, last_pbdat);
				button_state&=~(uint32)X_PRESSED;
				process_input(0, x_function, x_value);
			} else {
				//printf("X pressed %0x/%0x\n", current_pbdat, last_pbdat);
				button_state|=X_PRESSED;
				process_input(1, x_function, x_value);
			}
		}
		/* right shoulder button */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_R]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_R]) {
				button_state&=~(uint32)RSHOULDER_PRESSED;
				process_input(0, rshoulder_function, rshoulder_value);
			} else {
				button_state|=RSHOULDER_PRESSED;
				//if(!status_enabled) status_enabled=1;
				//else status_enabled=0;
				process_input(1, rshoulder_function, rshoulder_value);
			}
		}

		/* start */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_START]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_START]) {
				button_state&=~(uint32)START_PRESSED;
				process_input(0, start_function, start_value);
			} else {
				button_state|=START_PRESSED;
				//if(!options_enabled) options_enabled=1;
				//else options_enabled=0;
				process_input(1, start_function, start_value);
			}
		} 
		/* select */
		if((current_pbdat^last_pbdat)&keymap.scancode[EMU_KEY_SELECT]) {
			if(current_pbdat&keymap.scancode[EMU_KEY_SELECT]) {
				button_state&=~(uint32)SELECT_PRESSED;
				process_input(0, select_function, select_value);
			} else {
				button_state|=SELECT_PRESSED;
				process_input(1, select_function, select_value);
			}
		}
		last_pbdat=current_pbdat;
	}

//	if(chatboard_enabled) read_chatboard();
}

char kbd_feedbuf[255];
int kbd_feedbuf_pos;

void kbd_buf_feed(const char *s) {
	strcpy(kbd_feedbuf, s);
	kbd_feedbuf_pos=0;
}

void kbd_buf_update(C64 *TheC64) {
	if((kbd_feedbuf[kbd_feedbuf_pos]!=0)
			&& TheC64->RAM[198]==0) {
		TheC64->RAM[631]=kbd_feedbuf[kbd_feedbuf_pos];
		TheC64->RAM[198]=1;

		kbd_feedbuf_pos++;
	}
}

void load_prg(C64 *TheC64, const uint8 *prg, int prg_size) {
	uint8 start_hi, start_lo;
	uint16 start;
	int i;

	start_lo=*prg++;
	start_hi=*prg++;
	start=(start_hi<<8)+start_lo;

	for(i=0; i<(prg_size-2); i++) {
		TheC64->RAM[start+i]=prg[i];
	}
}
