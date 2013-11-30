
#ifndef _INPUT_H
#define _INPUT_H

//#include "libpogo/pogo.h"
#include "C64.h"

extern uint32 button_state;
extern uint32 joy_state;

#define START_PRESSED 1
#define SELECT_PRESSED 2
#define FIRE_PRESSED 4
#define X_PRESSED 4
#define O_PRESSED 8
#define LEFT_PRESSED 16
#define RIGHT_PRESSED 32
#define UP_PRESSED 64
#define DOWN_PRESSED 128
#define LSHOULDER_PRESSED 256
#define RSHOULDER_PRESSED 512

// button binding types
enum { KEY, JOY, UI, NUM_FUNCTIONS }; // input functions
enum { LEFT, RIGHT, UP, DOWN, FIRE, NUM_JOYVALS }; // joystick values
enum { MENU, VKEYB, STATS, NUM_UIVALS }; // ui values

struct keymapping {
	char *name;
	int key;
};

extern struct keymapping keybindings[];
extern int bkey_pressed;
extern int bkey_released;
extern int bkey;

extern int chatboard_enabled;
extern void enable_chatboard();
extern void disable_chatboard();

// button bindings
extern int start_function;
extern int start_value;
extern int select_function;
extern int select_value;
extern int x_function;
extern int x_value;
extern int o_function;
extern int o_value;
extern int left_function;
extern int left_value;
extern int right_function;
extern int right_value;
extern int up_function;
extern int up_value;
extern int down_function;
extern int down_value;
extern int lshoulder_function;
extern int lshoulder_value;
extern int rshoulder_function;
extern int rshoulder_value;

extern void poll_input(void);
extern void kbd_buf_feed(char *s);
extern void kbd_buf_update(C64 *TheC64);
extern void load_prg(C64 *TheC64, uint8 *prg, int prg_size);

#endif

