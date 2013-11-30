/*
 *  Display_SDL.i - C64 graphics display, emulator window handling,
 *                  SDL specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 */

#include "C64.h"
#include "SAM.h"
#include "Version.h"

#include <libgame.h>

#include "spmp_menu_input.h"
#include "spmp_menu.h"

// Colors for speedometer/drive LEDs
enum {
	black = 0,
	white = 1,
	fill_gray = 16,
	shine_gray = 17,
	shadow_gray = 18,
	red = 19,
	green = 20,
	PALETTE_SIZE = 21
};

/*
  C64 keyboard matrix:

    Bit 7   6   5   4   3   2   1   0
  0    CUD  F5  F3  F1  F7 CLR RET DEL
  1    SHL  E   S   Z   4   A   W   3
  2     X   T   F   C   6   D   R   5
  3     V   U   H   B   8   G   Y   7
  4     N   O   K   M   0   J   I   9
  5     ,   @   :   .   -   L   P   +
  6     /   ^   =  SHR HOM  ;   *   £
  7    R/S  Q   C= SPC  2  CTL  <-  1
*/

#define MATRIX(a,b) (((a) << 3) | (b))


emu_graph_params_t gp;
uint16_t _palette[256];
void *original_shadow;

emu_keymap_t keymap;
uint32_t keys;

static void map_buttons(void)
{
    libgame_buttonmap_t bm[] = {
        {"Start", EMU_KEY_START},
        {"Select", EMU_KEY_SELECT},	/* aka SELECT */
        {"O", EMU_KEY_O},		/* aka B */
        {"X", EMU_KEY_X},		/* aka X */
        {"Square", EMU_KEY_SQUARE},	/* aka A */
        {"Triangle", EMU_KEY_TRIANGLE},	/* aka Y */
        {"L", EMU_KEY_L},
        {"R", EMU_KEY_R},
        {0, 0}
    };
    libgame_map_buttons("frodo.map", &keymap, bm);
}

static int keystate[256];

int init_graphics(void)
{
	gp.pixels = (uint16_t *)malloc(DISPLAY_X * DISPLAY_Y);
	gp.width = DISPLAY_X;
	gp.height = DISPLAY_Y;
	gp.has_palette = 1;
	gp.palette = _palette;
	gp.src_clip_x = 0;
	gp.src_clip_y = 0;
	gp.src_clip_w = DISPLAY_X;
	gp.src_clip_h = DISPLAY_Y;
	emuIfGraphInit(&gp);
	/* Disable v-sync, we don't have that much time to waste. */
        original_shadow = gDisplayDev->getShadowBuffer();
        gDisplayDev->setShadowBuffer(gDisplayDev->getFrameBuffer());

        keymap.controller = 0;
        emuIfKeyInit(&keymap);
	if (libgame_load_buttons("frodo.map", &keymap) < 0) {
		map_buttons();
	}            
	for(int i=0; i<256; i++) keystate[i]=0;

	return 1;
}


/*
 *  Display constructor
 */

C64Display::C64Display(C64 *the_c64) : TheC64(the_c64)
{
#if 0
	quit_requested = false;
#endif
	speedometer_string[0] = 0;

}


/*
 *  Display destructor
 */

C64Display::~C64Display()
{
#if 0
	SDL_Quit();
#endif
}


/*
 *  Prefs may have changed
 */

void C64Display::NewPrefs(Prefs *prefs)
{
}


/*
 *  Redraw bitmap
 */

extern int keyboard_enabled;
extern int options_enabled;
extern int status_enabled;
void draw_ui(C64 *);

void C64Display::Update(void)
{
#if 0
	// Draw speedometer/LEDs
	SDL_Rect r = {0, DISPLAY_Y, DISPLAY_X, 15};
	SDL_FillRect(screen, &r, fill_gray);
	r.w = DISPLAY_X; r.h = 1;
	SDL_FillRect(screen, &r, shine_gray);
	r.y = DISPLAY_Y + 14;
	SDL_FillRect(screen, &r, shadow_gray);
	r.w = 16;
	for (int i=2; i<6; i++) {
		r.x = DISPLAY_X * i/5 - 24; r.y = DISPLAY_Y + 4;
		SDL_FillRect(screen, &r, shadow_gray);
		r.y = DISPLAY_Y + 10;
		SDL_FillRect(screen, &r, shine_gray);
	}
	r.y = DISPLAY_Y; r.w = 1; r.h = 15;
	for (int i=0; i<5; i++) {
		r.x = DISPLAY_X * i / 5;
		SDL_FillRect(screen, &r, shine_gray);
		r.x = DISPLAY_X * (i+1) / 5 - 1;
		SDL_FillRect(screen, &r, shadow_gray);
	}
	r.y = DISPLAY_Y + 4; r.h = 7;
	for (int i=2; i<6; i++) {
		r.x = DISPLAY_X * i/5 - 24;
		SDL_FillRect(screen, &r, shadow_gray);
		r.x = DISPLAY_X * i/5 - 9;
		SDL_FillRect(screen, &r, shine_gray);
	}
	r.y = DISPLAY_Y + 5; r.w = 14; r.h = 5;
	for (int i=0; i<4; i++) {
		r.x = DISPLAY_X * (i+2) / 5 - 23;
		int c;
		switch (led_state[i]) {
			case LED_ON:
				c = green;
				break;
			case LED_ERROR_ON:
				c = red;
				break;
			default:
				c = black;
				break;
		}
		SDL_FillRect(screen, &r, c);
	}

	draw_string(screen, DISPLAY_X * 1/5 + 8, DISPLAY_Y + 4, "D\x12 8", black, fill_gray);
	draw_string(screen, DISPLAY_X * 2/5 + 8, DISPLAY_Y + 4, "D\x12 9", black, fill_gray);
	draw_string(screen, DISPLAY_X * 3/5 + 8, DISPLAY_Y + 4, "D\x12 10", black, fill_gray);
	draw_string(screen, DISPLAY_X * 4/5 + 8, DISPLAY_Y + 4, "D\x12 11", black, fill_gray);
	draw_string(screen, 24, DISPLAY_Y + 4, speedometer_string, black, fill_gray);

	// Update display
	SDL_Flip(screen);
#else
	//printf("display update\n");
	draw_string(24, DISPLAY_Y - 8, speedometer_string, black, fill_gray);
	//printf("speed %s\n", speedometer_string);
                if(keyboard_enabled||options_enabled||status_enabled)
                                        draw_ui(TheC64);
	emuIfGraphShow();
#endif
}


#if 1
/*
 *  Draw string into surface using the C64 ROM font
 */

void C64Display::draw_string(int x, int y, const char *str, uint8 front_color, uint8 back_color)
{
	uint8 *pb = (uint8 *)gp.pixels + DISPLAY_X*y + x;
	char c;
	while ((c = *str++) != 0) {
		uint8 *q = TheC64->Char + c*8 + 0x800;
		uint8 *p = pb;
		for (int y=0; y<8; y++) {
			uint8 v = *q++;
			p[0] = (v & 0x80) ? front_color : back_color;
			p[1] = (v & 0x40) ? front_color : back_color;
			p[2] = (v & 0x20) ? front_color : back_color;
			p[3] = (v & 0x10) ? front_color : back_color;
			p[4] = (v & 0x08) ? front_color : back_color;
			p[5] = (v & 0x04) ? front_color : back_color;
			p[6] = (v & 0x02) ? front_color : back_color;
			p[7] = (v & 0x01) ? front_color : back_color;
			p += DISPLAY_X;
		}
		pb += 8;
	}
}
#endif


#if 0
/*
 *  LED error blink
 */

void C64Display::pulse_handler(...)
{
	for (int i=0; i<4; i++)
		switch (c64_disp->led_state[i]) {
			case LED_ERROR_ON:
				c64_disp->led_state[i] = LED_ERROR_OFF;
				break;
			case LED_ERROR_OFF:
				c64_disp->led_state[i] = LED_ERROR_ON;
				break;
		}
}
#endif


#if 1
/*
 *  Draw speedometer
 */

void C64Display::Speedometer(int speed)
{
	static int delay = 0;
	//printf("setting speedo %d\n", speed);
	if (delay >= 20) {
		delay = 0;
		sprintf(speedometer_string, "%d%%", speed);
	} else
		delay++;
}
#endif


/*
 *  Return pointer to bitmap data
 */

uint8 *C64Display::BitmapBase(void)
{
	return (uint8 *)gp.pixels;
}


/*
 *  Return number of bytes per row
 */

int C64Display::BitmapXMod(void)
{
	return DISPLAY_X;
}


#if 0
/*
 *  Poll the keyboard
 */

static void translate_key(SDLKey key, bool key_up, uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick)
{
	int c64_key = -1;
	switch (key) {
		case SDLK_a: c64_key = MATRIX(1,2); break;
		case SDLK_b: c64_key = MATRIX(3,4); break;
		case SDLK_c: c64_key = MATRIX(2,4); break;
		case SDLK_d: c64_key = MATRIX(2,2); break;
		case SDLK_e: c64_key = MATRIX(1,6); break;
		case SDLK_f: c64_key = MATRIX(2,5); break;
		case SDLK_g: c64_key = MATRIX(3,2); break;
		case SDLK_h: c64_key = MATRIX(3,5); break;
		case SDLK_i: c64_key = MATRIX(4,1); break;
		case SDLK_j: c64_key = MATRIX(4,2); break;
		case SDLK_k: c64_key = MATRIX(4,5); break;
		case SDLK_l: c64_key = MATRIX(5,2); break;
		case SDLK_m: c64_key = MATRIX(4,4); break;
		case SDLK_n: c64_key = MATRIX(4,7); break;
		case SDLK_o: c64_key = MATRIX(4,6); break;
		case SDLK_p: c64_key = MATRIX(5,1); break;
		case SDLK_q: c64_key = MATRIX(7,6); break;
		case SDLK_r: c64_key = MATRIX(2,1); break;
		case SDLK_s: c64_key = MATRIX(1,5); break;
		case SDLK_t: c64_key = MATRIX(2,6); break;
		case SDLK_u: c64_key = MATRIX(3,6); break;
		case SDLK_v: c64_key = MATRIX(3,7); break;
		case SDLK_w: c64_key = MATRIX(1,1); break;
		case SDLK_x: c64_key = MATRIX(2,7); break;
		case SDLK_y: c64_key = MATRIX(3,1); break;
		case SDLK_z: c64_key = MATRIX(1,4); break;

		case SDLK_0: c64_key = MATRIX(4,3); break;
		case SDLK_1: c64_key = MATRIX(7,0); break;
		case SDLK_2: c64_key = MATRIX(7,3); break;
		case SDLK_3: c64_key = MATRIX(1,0); break;
		case SDLK_4: c64_key = MATRIX(1,3); break;
		case SDLK_5: c64_key = MATRIX(2,0); break;
		case SDLK_6: c64_key = MATRIX(2,3); break;
		case SDLK_7: c64_key = MATRIX(3,0); break;
		case SDLK_8: c64_key = MATRIX(3,3); break;
		case SDLK_9: c64_key = MATRIX(4,0); break;

		case SDLK_SPACE: c64_key = MATRIX(7,4); break;
		case SDLK_BACKQUOTE: c64_key = MATRIX(7,1); break;
		case SDLK_BACKSLASH: c64_key = MATRIX(6,6); break;
		case SDLK_COMMA: c64_key = MATRIX(5,7); break;
		case SDLK_PERIOD: c64_key = MATRIX(5,4); break;
		case SDLK_MINUS: c64_key = MATRIX(5,0); break;
		case SDLK_EQUALS: c64_key = MATRIX(5,3); break;
		case SDLK_LEFTBRACKET: c64_key = MATRIX(5,6); break;
		case SDLK_RIGHTBRACKET: c64_key = MATRIX(6,1); break;
		case SDLK_SEMICOLON: c64_key = MATRIX(5,5); break;
		case SDLK_QUOTE: c64_key = MATRIX(6,2); break;
		case SDLK_SLASH: c64_key = MATRIX(6,7); break;

		case SDLK_ESCAPE: c64_key = MATRIX(7,7); break;
		case SDLK_RETURN: c64_key = MATRIX(0,1); break;
		case SDLK_BACKSPACE: case SDLK_DELETE: c64_key = MATRIX(0,0); break;
		case SDLK_INSERT: c64_key = MATRIX(6,3); break;
		case SDLK_HOME: c64_key = MATRIX(6,3); break;
		case SDLK_END: c64_key = MATRIX(6,0); break;
		case SDLK_PAGEUP: c64_key = MATRIX(6,0); break;
		case SDLK_PAGEDOWN: c64_key = MATRIX(6,5); break;

		case SDLK_LCTRL: case SDLK_TAB: c64_key = MATRIX(7,2); break;
		case SDLK_RCTRL: c64_key = MATRIX(7,5); break;
		case SDLK_LSHIFT: c64_key = MATRIX(1,7); break;
		case SDLK_RSHIFT: c64_key = MATRIX(6,4); break;
		case SDLK_LALT: case SDLK_LMETA: c64_key = MATRIX(7,5); break;
		case SDLK_RALT: case SDLK_RMETA: c64_key = MATRIX(7,5); break;

		case SDLK_UP: c64_key = MATRIX(0,7)| 0x80; break;
		case SDLK_DOWN: c64_key = MATRIX(0,7); break;
		case SDLK_LEFT: c64_key = MATRIX(0,2) | 0x80; break;
		case SDLK_RIGHT: c64_key = MATRIX(0,2); break;

		case SDLK_F1: c64_key = MATRIX(0,4); break;
		case SDLK_F2: c64_key = MATRIX(0,4) | 0x80; break;
		case SDLK_F3: c64_key = MATRIX(0,5); break;
		case SDLK_F4: c64_key = MATRIX(0,5) | 0x80; break;
		case SDLK_F5: c64_key = MATRIX(0,6); break;
		case SDLK_F6: c64_key = MATRIX(0,6) | 0x80; break;
		case SDLK_F7: c64_key = MATRIX(0,3); break;
		case SDLK_F8: c64_key = MATRIX(0,3) | 0x80; break;

		case SDLK_KP0: case SDLK_KP5: c64_key = 0x10 | 0x40; break;
		case SDLK_KP1: c64_key = 0x06 | 0x40; break;
		case SDLK_KP2: c64_key = 0x02 | 0x40; break;
		case SDLK_KP3: c64_key = 0x0a | 0x40; break;
		case SDLK_KP4: c64_key = 0x04 | 0x40; break;
		case SDLK_KP6: c64_key = 0x08 | 0x40; break;
		case SDLK_KP7: c64_key = 0x05 | 0x40; break;
		case SDLK_KP8: c64_key = 0x01 | 0x40; break;
		case SDLK_KP9: c64_key = 0x09 | 0x40; break;

		case SDLK_KP_DIVIDE: c64_key = MATRIX(6,7); break;
		case SDLK_KP_ENTER: c64_key = MATRIX(0,1); break;
	}

	if (c64_key < 0)
		return;

	// Handle joystick emulation
	if (c64_key & 0x40) {
		c64_key &= 0x1f;
		if (key_up)
			*joystick |= c64_key;
		else
			*joystick &= ~c64_key;
		return;
	}

	// Handle other keys
	bool shifted = c64_key & 0x80;
	int c64_byte = (c64_key >> 3) & 7;
	int c64_bit = c64_key & 7;
	if (key_up) {
		if (shifted) {
			key_matrix[6] |= 0x10;
			rev_matrix[4] |= 0x40;
		}
		key_matrix[c64_byte] |= (1 << c64_bit);
		rev_matrix[c64_bit] |= (1 << c64_byte);
	} else {
		if (shifted) {
			key_matrix[6] &= 0xef;
			rev_matrix[4] &= 0xbf;
		}
		key_matrix[c64_byte] &= ~(1 << c64_bit);
		rev_matrix[c64_bit] &= ~(1 << c64_byte);
	}
}
#endif

void KeyPress(int key, uint8 *key_matrix, uint8 *rev_matrix) {
	int c64_byte, c64_bit, shifted;
	if(!keystate[key]) {
		keystate[key]=1;
		c64_byte=key>>3;
		c64_bit=key&7;
		shifted=key&128;
		c64_byte&=7;
		if(shifted) {
			key_matrix[6] &= 0xef;
			rev_matrix[4] &= 0xbf;
		}
		key_matrix[c64_byte]&=~(1<<c64_bit);
		rev_matrix[c64_bit]&=~(1<<c64_byte);
	}
}

void KeyRelease(int key, uint8 *key_matrix, uint8 *rev_matrix) {
	int c64_byte, c64_bit, shifted;
	if(keystate[key]) {
		keystate[key]=0;
		c64_byte=key>>3;
		c64_bit=key&7;
		shifted=key&128;
		c64_byte&=7;
		if(shifted) {
			key_matrix[6] |= 0x10;
			rev_matrix[4] |= 0x40;
		}
		key_matrix[c64_byte]|=(1<<c64_bit);
		rev_matrix[c64_bit]|=(1<<c64_byte);
	}
}

extern void poll_input(void);

void C64Display::PollKeyboard(uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick)
{
	//printf("poll keyboard sf %d\n", ThePrefs.SkipFrames);
	ge_key_data_t bogus_keys;
	NativeGE_getKeyInput4Ntv(&bogus_keys);
	//keys = emuIfKeyGetInput(&keymap);
	poll_input();
	kbd_buf_update(TheC64);

	// check button-mapped keys
	if(bkey_pressed) {
		//printf("bkey %d pressed\n", bkey);
		KeyPress(bkey, key_matrix, rev_matrix);
		bkey_pressed=0;
	}
	if(bkey_released) {
		//printf("bkey %d released\n", bkey);
		KeyRelease(bkey, key_matrix, rev_matrix);
		bkey_released=0;
	}

	// check virtual keyboard
	if(keyboard_enabled) {
		if(vkey_pressed) {
			//printf("vkey %d pressed\n", vkey);
			KeyPress(vkey, key_matrix, rev_matrix);
			vkey_pressed=0;
		}
		if(vkey_released) {
			//printf("vkey %d released\n", vkey);
			KeyRelease(vkey, key_matrix, rev_matrix);
			vkey_released=0;
			//vkey=0;
		}
	}

	//memset(key_matrix, 0xff, 8);
	//memset(rev_matrix, 0xff, 8);
	//*joystick = 0xff;
}


/*
 *  Check if NumLock is down (for switching the joystick keyboard emulation)
 */

bool C64Display::NumLock(void)
{
//	return num_locked;
	return false;
}


/*
 *  Allocate C64 colors
 */

void C64Display::InitColors(uint8 *colors)
{
	for (int i=0; i<16; i++) {
		_palette[i] = MAKE_RGB565(palette_red[i], palette_green[i], palette_blue[i]);
	}
	_palette[fill_gray] = MAKE_RGB565(0xd0, 0xd0, 0xd0);
	_palette[shine_gray] = MAKE_RGB565(0xf0, 0xf0, 0xf0);
	_palette[shadow_gray] = MAKE_RGB565(0x80, 0x80, 0x80);

	for (int i=0; i<256; i++)
		colors[i] = i & 0x0f;
}


/*
 *  Show a requester (error message)
 */

long ShowRequester(const char *a,const char *b,const char *)
{
	printf("%s: %s\n", a, b);
	return 1;
}
