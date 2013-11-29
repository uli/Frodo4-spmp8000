
#ifndef _MENU_H
#define _MENU_H

//#include "pogo.h"
#include "C64.h"

#define FONT_WIDTH 8
#define FONT_HEIGHT 8

#define BUFFER_WIDTH 384
//#define BUFFER_HEIGHT 272
//#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
#define BORDER_LEFT (BUFFER_WIDTH-DISPLAY_WIDTH)/2
#define BORDER_TOP (BUFFER_HEIGHT-DISPLAY_HEIGHT)/2

extern uint8 *emu_screen;

extern int vkey_pressed;
extern int vkey_released;
extern int vkey;

extern void draw_ui(C64 *);

#endif

