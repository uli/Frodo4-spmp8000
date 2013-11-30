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
	speedometer_string[0] = 0;
}


/*
 *  Display destructor
 */

C64Display::~C64Display()
{
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
	//printf("display update\n");
	draw_string(24, DISPLAY_Y - 8, speedometer_string, black, fill_gray);
	//printf("speed %s\n", speedometer_string);
                if(keyboard_enabled||options_enabled||status_enabled)
                                        draw_ui(TheC64);
	emuIfGraphShow();
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
