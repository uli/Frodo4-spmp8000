
#include <libgame.h>

#if 0
extern "C" {
#include "input.h"
#include "pogo.h"
#include "ui.h"
#include "gpmisc.h"
#include "menu.h"
#include "c64/c64loader.h"
}
#else
#include "sysdeps.h"
#include <stdio.h>
#include "Display.h"
#include "spmp_menu.h"
#include "spmp_menu_input.h"
#endif

#include "C64.h"
#include "Prefs.h"
//#include "SAM.h"

#define opendir _ecos_opendir
#define DIR _ecos_DIR
#define dirent _ecos_dirent
#define readdir _ecos_readdir
#define closedir _ecos_closedir

extern emu_graph_params_t gp;

int emu_paused=0;

int keyboard_enabled=0;
int options_enabled=0;
int status_enabled=0;

int emu_speed;
int emu_minutes;
int emu_seconds;
int emu_fps;

int drive8active=0;

int brightness=0;
int contrast=18;
float fcontrast=0;

extern uint32_t keys;
extern emu_keymap_t keymap;


extern Prefs ThePrefs;
extern int current_joystick;

//extern uint8 charrom[];
uint8 *font;

#define MATRIX(a,b) (((a) << 3) | (b))
int vkey_pressed=0;
int vkey_released=0;
int vkey=0;

int cursor_x=15;
int cursor_x_last=15;
int cursor_y=20;
int cursor_y_last=20;

struct dir_item {
	char *name;
	int type;
};

int ascii2charrom(char chr) {
	int x=(int)chr;

	if(x>=97&&x<=122) x+=160;
	else if(x>=65&&x<=90) x-=64;
	return x;
}

void text_out8(uint8 *screen, const char *string, uint8 bg, uint8 fg, 
		int x, int y, int transparant) {
	//printf("text_out %p %s %d %d at %d/%d t %d\n", screen, string, bg, fg, x, y, transparant);

	x *= 8;
	y *= 8;

	uint8 *pb = (uint8 *)screen + DISPLAY_X*y + x;
	char c;
	while ((c = *string++) != 0) {
		uint8 *q = font + ascii2charrom(c)*8;
		uint8 *p = pb;
		for (int y=0; y<8; y++) {
			uint8 v = *q++;
			p[0] = (v & 0x80) ? fg : bg;
			p[1] = (v & 0x40) ? fg : bg;
			p[2] = (v & 0x20) ? fg : bg;
			p[3] = (v & 0x10) ? fg : bg;
			p[4] = (v & 0x08) ? fg : bg;
			p[5] = (v & 0x04) ? fg : bg;
			p[6] = (v & 0x02) ? fg : bg;
			p[7] = (v & 0x01) ? fg : bg;
			p += DISPLAY_X;
		}
		pb += 8;
	}
#if 0
	int char_width=DISPLAY_X/FONT_WIDTH;
	int xpos;
	int xx;
	int yy;
	uint8 pixel;
	unsigned char *character_data;
	int i;
	int screen_x, screen_y;

	if(y>=(DISPLAY_HEIGHT/FONT_HEIGHT)) return;
	xpos=x;
	for(i=0; (xpos<char_width)&&string[i]; i++) {
		character_data=font+(ascii2charrom(string[i])
				*((FONT_WIDTH/8)*FONT_HEIGHT));
		xx=0;
		yy=0;
		while(yy<FONT_HEIGHT) {
			pixel=*(character_data+yy);
			pixel=pixel>>((FONT_WIDTH-1)-xx);
			if(pixel&1) {
				pixel=fg;
			} else {
				pixel=bg;
			}
			if(!(transparant&&(pixel==bg)&&(xx&1)&&(yy&1))) {
				screen_x=(BORDER_LEFT+(xpos*FONT_WIDTH))+xx;
				screen_y=(BORDER_TOP+(y*FONT_HEIGHT))+yy;
				*(screen+(BUFFER_HEIGHT*screen_x)
						+((BUFFER_HEIGHT-1)-screen_y))
						=pixel;
			}

			xx++;
			if(xx==FONT_WIDTH) {
				xx=0;
				yy++;
			}
		}
		xpos++;
	}
#endif
}

void sort_dir(struct dir_item *list, int num_items, int sepdir) {
	int i;
	struct dir_item temp;

	for(i=0; i<(num_items-1); i++) {
		if(strcmp(list[i].name, list[i+1].name)>0) {
			temp=list[i];
			list[i]=list[i+1];
			list[i+1]=temp;
			i=0;
		}
	}
	if(sepdir) {
		for(i=0; i<(num_items-1); i++) {
			if((list[i].type==1)&&(list[i+1].type==0)) {
				temp=list[i];
				list[i]=list[i+1];
				list[i+1]=temp;
				i=0;
			}
		}
	}
}

char *filerequest(char *dir) {
	static const char *cwd=NULL;
	static int num_items=0;
	struct dir_item dir_items[1024];
	struct stat item;
	static int fcursor_pos;
	static int first_visible;
	static int row, column;
	DIR *dirstream;
	struct dirent *direntry;
	char *path;
	uint16 bg,fg;
	int i;
	int pathlength;
	const char *blank_line="                      ";
	int menu_x=5;
	int menu_y=3;
	int menu_height=25;
	static int is_moving=0;
	static int threshold=10;
	static int stepping=2;

	uint8 menu_bg=14;
	uint8 menu_fg=6;
	uint8 menu_selbg=7;
	uint8 menu_selfg=6;

	if(dir!=NULL) cwd=dir;
	if(cwd==NULL) cwd="frodo";

	if(num_items==0) {
		dirstream=opendir(cwd);
		if(dirstream==NULL) {
			button_state&=~X_PRESSED;
			printf("error opening directory %s\n", cwd);
			return (char *)-1;
		}
		// read directory entries
		while(direntry=readdir(dirstream)) {
			dir_items[num_items].name=(char *)malloc(
					strlen(direntry->d_name)+1);
			strcpy(dir_items[num_items].name, direntry->d_name);
			printf("found file -%s-\n", direntry->d_name);
			num_items++;
			if(num_items>1024) break;
		}
		closedir(dirstream);
		// get entry types
		for(i=0; i<num_items; i++) {
			path=(char *)malloc(strlen(cwd)
					+strlen(dir_items[i].name)
					+2);
			sprintf(path, "%s/%s", cwd, dir_items[i].name);
			if(!stat(path, &item)) {
				if(S_ISDIR(item.st_mode)) {
					dir_items[i].type=0;
				} else {
					dir_items[i].type=1;
				}
			} else {
				dir_items[i].type=0;
			}
			free(path);
		}
		sort_dir(dir_items, num_items, 1);
		fcursor_pos=0;
		first_visible=0;
	}

	// display current directory
	text_out8((uint8 *)gp.pixels, cwd+6, menu_bg, menu_fg, 5, menu_y-1, 0);

	// display directory contents
	row=0;
	while(row<num_items
			//&& row<(DISPLAY_HEIGHT/FONT_HEIGHT)) {
			&& row<menu_height) {
		column=menu_x;
		if(row==(fcursor_pos-first_visible)) {
			bg=menu_selbg;
			fg=menu_selfg;
		} else {
			bg=menu_bg;
			fg=menu_fg;
		}
		if(dir_items[row+first_visible].type==0) {
			text_out8((uint8 *)gp.pixels, 
					"<DIR> ", 
				bg, fg, column, menu_y+row, 0);
		} else {
			text_out8((uint8 *)gp.pixels, 
					"      ", 
				bg, fg, column, menu_y+row, 0);
		}
		column+=6;
		text_out8((uint8 *)gp.pixels, blank_line, 
				bg, fg, column, menu_y+row, 0);
		text_out8((uint8 *)gp.pixels, dir_items[row+first_visible].name, 
				bg, fg, column, menu_y+row, 0);
		row++;
	}
	while(row<menu_height) {
		text_out8((uint8 *)gp.pixels, "      ", 
			menu_bg, menu_fg, menu_x, menu_y+row, 0);
		text_out8((uint8 *)gp.pixels, blank_line, 
			menu_bg, menu_fg, column, menu_y+row, 0);
		row++;
	}
	if(button_state&DOWN_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(fcursor_pos<(num_items-1)) fcursor_pos++;
			if((fcursor_pos-first_visible) >=menu_height) first_visible++;
		}
		is_moving++;
	} else if(button_state&UP_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(fcursor_pos>0) fcursor_pos--;
			if(fcursor_pos<first_visible) first_visible--;
		}
		is_moving++;
	} else {
		is_moving=0;
	}
	
	if(button_state&X_PRESSED) {
		button_state&=~X_PRESSED;
		path=(char *)malloc(strlen(cwd)
				+strlen(dir_items[fcursor_pos].name)
				+2);
		sprintf(path, "%s/%s", cwd, dir_items[fcursor_pos].name);
		for(i=0; i<num_items; i++) free(dir_items[i].name);
		num_items=0;
		if(dir_items[fcursor_pos].type==0) {
			// directory selected
			pathlength=strlen(path);
			if(path[pathlength-1]=='.'
				// check for . selected
					&& path[pathlength-2]=='/') {
				path[pathlength-2]='\0';
				return path;
			} else if(path[pathlength-1]=='.'
				// check for .. selected
					&& path[pathlength-2]=='.'
					&& path[pathlength-3]=='/'
					&&pathlength>8) {
				for(i=4; path[pathlength-i]!='/'; i++);
				path[pathlength-i]='\0';
				if(strlen(path)<=5) {
					path[5]='/';
					path[6]='\0';
				}
				cwd=path;
			} else {
				cwd=path;
			}
		} else {
			// file selected
			printf("selected file -%s-\n", path);
			return path;
		}
	} else if(button_state&O_PRESSED) {
		return (char *)-1;
	}
	return NULL;
}

void draw_buttonmap(int x, int y, int function, int value) {
	int bg, fg;
	uint8 menu_bg=14;
	uint8 menu_fg=6;
	uint8 menu_selbg=7;
	uint8 menu_selfg=6;
	const char *value_str=NULL;

	if(function==UI) { 
		bg=menu_selbg; fg=menu_selfg;
		switch(value) {
			case MENU:
				value_str="menu";
				break;
			case VKEYB:
				value_str="keyboard";
				break;
			case STATS:
				value_str="stats";
				break;
		}
	} else { 
		bg=menu_bg; fg=menu_fg;
	}
	text_out8((uint8 *)gp.pixels, "ui ", bg, fg, x, y, 0);
	if(function==KEY) { 
		bg=menu_selbg; fg=menu_selfg;
		value_str=keybindings[value].name;
	} else { 
		bg=menu_bg; fg=menu_fg;
	}
	text_out8((uint8 *)gp.pixels, "key ", bg, fg, x+3, y, 0);
	if(function==JOY) { 
		bg=menu_selbg; fg=menu_selfg;
		switch(value) {
			case LEFT:
				value_str="left";
				break;
			case RIGHT:
				value_str="right";
				break;
			case UP:
				value_str="up";
				break;
			case DOWN:
				value_str="down";
				break;
			case FIRE:
				value_str="fire";
				break;
		}
	} else { 
		bg=menu_bg; fg=menu_fg;
	}
	text_out8((uint8 *)gp.pixels, "joy", bg, fg, x+7, y, 0);

	text_out8((uint8 *)gp.pixels, value_str, menu_bg, menu_fg, x, y+1, 0);
}

void button_dec_function(int *function, int *value) {
	if(*function==KEY) {
		*function=UI;
		*value=0;
	} else if(*function==UI) {
		*function=JOY;
		*value=0;
	} else {
		*function=KEY;
		*value=0;
	}
}
void button_dec_value(int *function, int *value) {
	(*value)--;
	if(*value<0) *value=0;
}
void button_inc_function(int *function, int *value) {
	if(*function==KEY) {
		*function=JOY;
		*value=0;
	} else if(*function==JOY) {
		*function=UI;
		*value=0;
	} else {
		*function=KEY;
		*value=0;
	}
}
void button_inc_value(int *function, int *value) {
	(*value)++;
	if(*function==KEY) {
		if(keybindings[*value].name==NULL) (*value)--;
	} else if(*function==JOY) {
		if(*value>=NUM_JOYVALS) (*value)--;
	} else if(*function==UI) {
		if(*value>=NUM_UIVALS) (*value)--;
	}
}

int draw_snapshotui(C64 *TheC64, int save, char *cur_image) {
	static int menu_y=3;
	static int menu_x=5;
	static int cursor_pos=0;
	int i, j;
	uint16 bg, fg;
	uint8 menu_bg=14;
	uint8 menu_fg=6;
	uint8 menu_selbg=7;
	uint8 menu_selfg=6;
#define NUM_SLOTS 26
#define MENU_WIDTH 28
	char option_txt[MENU_WIDTH+1];
	char filename[255];
	static int is_moving=0;
	static int threshold=10;
	static int stepping=2;
	static char *snapshot_index=NULL;
	FILE *index_file;
	const char *index_filename="frodo/snap.idx";

	if(!snapshot_index) {
		snapshot_index=(char *)malloc(NUM_SLOTS*MENU_WIDTH);
		index_file=fopen(index_filename, "r");
		if(!index_file) {
			for(i=0; i<NUM_SLOTS*MENU_WIDTH; i++) snapshot_index[i]=' ';
		} else {
			fread(snapshot_index, NUM_SLOTS*MENU_WIDTH, 1, index_file);
			fclose(index_file);
		}
	}

	for(i=0; i<NUM_SLOTS; i++) {
		if(cursor_pos==i) { 
			bg=menu_selbg;
			fg=menu_selfg;
		} else { 
			bg=menu_bg;
			fg=menu_fg;
		}
		for(j=0; j<MENU_WIDTH; j++) {
			option_txt[+j]=snapshot_index[(MENU_WIDTH*i)+j];
		}
		option_txt[MENU_WIDTH]='\0';
		text_out8((uint8 *)gp.pixels, option_txt, bg, fg, menu_x, menu_y+i, 0);
	}

	if(button_state&UP_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_pos>0) cursor_pos--;
		}
		is_moving++;
	} else if(button_state&DOWN_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_pos<(NUM_SLOTS-1)) cursor_pos++;
		}
		is_moving++;
	} else {
		is_moving=0;
	}

	if(button_state&O_PRESSED) return -1;

	if(button_state&X_PRESSED) {
		sprintf(filename, "frodo/snap%d", cursor_pos);
		if(save) {
			remove(filename);
			TheC64->SaveSnapshot(filename);

			for(i=strlen(cur_image); i>0 && cur_image[i]!='/'; i--);
			if(cur_image[i]=='/') i++;
			j=cursor_pos*MENU_WIDTH;
			while(cur_image[i] && cur_image[i]!='.') {
				snapshot_index[j++]=cur_image[i++];
			}
			remove(index_filename);
			index_file=fopen(index_filename, "w");
			fwrite(snapshot_index, NUM_SLOTS*MENU_WIDTH, 1, index_file);
			fclose(index_file);
		} else {
			TheC64->LoadSnapshot(filename);
		}
		return 1;
	}

	return 0;
}

int draw_buttonsui() {
	int i;
	uint16 bg, fg;
	static int cursor_pos=3;
	static int menu_y=3;
	static int menu_x=5;
	uint8 menu_bg=14;
	uint8 menu_fg=6;
	uint8 menu_selbg=7;
	uint8 menu_selfg=6;
	static int is_moving=0;
	static int threshold=10;
	static int stepping=2;
	static int dec_option=0;
	static int inc_option=0;

	enum {
		EXIT, BLANK1,
		BUTTON_X, BUTTON_XV, 
		BUTTON_O, BUTTON_OV,
		JOYPAD_LEFT, JOYPAD_LEFTV, 
		JOYPAD_RIGHT, JOYPAD_RIGHTV, 
		JOYPAD_UP, JOYPAD_UPV, 
		JOYPAD_DOWN, JOYPAD_DOWNV,
		SHOULDER_LEFT, SHOULDER_LEFTV, 
		SHOULDER_RIGHT, SHOULDER_RIGHTV,
		START_BUTTON, START_BUTTONV, 
		SELECT_BUTTON, SELECT_BUTTONV,
		NUM_OPTIONS
	};

	const char *option_txt[NUM_OPTIONS+1];
	option_txt[EXIT]=		"exit...                     ";
	option_txt[BLANK1]=		"                            ";
	option_txt[JOYPAD_LEFT]=	"Joypad left                 ";
	option_txt[JOYPAD_LEFTV]=	"                            ";
	option_txt[JOYPAD_RIGHT]=	"Joypad right                ";
	option_txt[JOYPAD_RIGHTV]=	"                            ";
	option_txt[JOYPAD_UP]=		"Joypad up                   ";
	option_txt[JOYPAD_UPV]=		"                            ";
	option_txt[JOYPAD_DOWN]=	"Joypad down                 ";
	option_txt[JOYPAD_DOWNV]=	"                            ";
	option_txt[BUTTON_X]=		"Button X                    ";
	option_txt[BUTTON_XV]=		"                            ";
	option_txt[BUTTON_O]=		"Button O                    ";
	option_txt[BUTTON_OV]=		"                            ";
	option_txt[SHOULDER_LEFT]=	"Left shoulder               ";
	option_txt[SHOULDER_LEFTV]=	"                            ";
	option_txt[SHOULDER_RIGHT]=	"Right shoulder              ";
	option_txt[SHOULDER_RIGHTV]=	"                            ";
	option_txt[START_BUTTON]=	"Start                       ";
	option_txt[START_BUTTONV]=	"                            ";
	option_txt[SELECT_BUTTON]=	"Select                      ";
	option_txt[SELECT_BUTTONV]=	"                            ";

	for(i=0; option_txt[i]; i++) {
		if(cursor_pos==(menu_y+i)) { 
			bg=menu_selbg;
			fg=menu_selfg;
		} else { 
			bg=menu_bg;
			fg=menu_fg;
		}
		text_out8((uint8 *)gp.pixels, option_txt[i], bg, fg, menu_x, menu_y+i, 0);
	}
	draw_buttonmap(18, menu_y+JOYPAD_LEFT, left_function, left_value);
	draw_buttonmap(18, menu_y+JOYPAD_RIGHT, right_function, right_value);
	draw_buttonmap(18, menu_y+JOYPAD_UP, up_function, up_value);
	draw_buttonmap(18, menu_y+JOYPAD_DOWN, down_function, down_value);
	draw_buttonmap(18, menu_y+BUTTON_X, x_function, x_value);
	draw_buttonmap(18, menu_y+BUTTON_O, o_function, o_value);
	draw_buttonmap(18, menu_y+SHOULDER_LEFT, lshoulder_function, lshoulder_value);
	draw_buttonmap(18, menu_y+SHOULDER_RIGHT, rshoulder_function, rshoulder_value);
	draw_buttonmap(18, menu_y+START_BUTTON, start_function, start_value);
	draw_buttonmap(18, menu_y+SELECT_BUTTON, select_function, select_value);

	if(button_state&UP_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_pos>menu_y) cursor_pos--;
		}
		is_moving++;
	} else if(button_state&DOWN_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_pos<menu_y+NUM_OPTIONS-1) cursor_pos++;
		}
		is_moving++;
	} else if(button_state&LEFT_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			dec_option=1;
		}
		is_moving++;
	} else if(button_state&RIGHT_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			inc_option=1;
		}
		is_moving++;
	} else {
		is_moving=0;
	}

	if(dec_option) {
		dec_option=0;
		switch(cursor_pos) {
			case JOYPAD_LEFT+3:
				button_dec_function(&left_function, &left_value);
				break;
			case JOYPAD_LEFTV+3:
				button_dec_value(&left_function, &left_value);
				break;
			case JOYPAD_RIGHT+3:
				button_dec_function(&right_function, &right_value);
				break;
			case JOYPAD_RIGHTV+3:
				button_dec_value(&right_function, &right_value);
				break;
			case JOYPAD_UP+3:
				button_dec_function(&up_function, &up_value);
				break;
			case JOYPAD_UPV+3:
				button_dec_value(&up_function, &up_value);
				break;
			case JOYPAD_DOWN+3:
				button_dec_function(&down_function, &down_value);
				break;
			case JOYPAD_DOWNV+3:
				button_dec_value(&down_function, &down_value);
				break;
			case BUTTON_X+3:
				button_dec_function(&x_function, &x_value);
				break;
			case BUTTON_XV+3:
				button_dec_value(&x_function, &x_value);
				break;
			case BUTTON_O+3:
				button_dec_function(&o_function, &o_value);
				break;
			case BUTTON_OV+3:
				button_dec_value(&o_function, &o_value);
				break;
			case SHOULDER_LEFT+3:
				button_dec_function(&lshoulder_function, &lshoulder_value);
				break;
			case SHOULDER_LEFTV+3:
				button_dec_value(&lshoulder_function, &lshoulder_value);
				break;
			case SHOULDER_RIGHT+3:
				button_dec_function(&rshoulder_function, &rshoulder_value);
				break;
			case SHOULDER_RIGHTV+3:
				button_dec_value(&rshoulder_function, &rshoulder_value);
				break;
			case START_BUTTON+3:
				button_dec_function(&start_function, &start_value);
				break;
			case START_BUTTONV+3:
				button_dec_value(&start_function, &start_value);
				break;
			case SELECT_BUTTON+3:
				button_dec_function(&select_function, &select_value);
				break;
			case SELECT_BUTTONV+3:
				button_dec_value(&select_function, &select_value);
				break;
		}
	} else if(inc_option) {
		inc_option=0;
		switch(cursor_pos) {
			case JOYPAD_LEFT+3:
				button_inc_function(&left_function, &left_value);
				break;
			case JOYPAD_LEFTV+3:
				button_inc_value(&left_function, &left_value);
				break;
			case JOYPAD_RIGHT+3:
				button_inc_function(&right_function, &right_value);
				break;
			case JOYPAD_RIGHTV+3:
				button_inc_value(&right_function, &right_value);
				break;
			case JOYPAD_UP+3:
				button_inc_function(&up_function, &up_value);
				break;
			case JOYPAD_UPV+3:
				button_inc_value(&up_function, &up_value);
				break;
			case JOYPAD_DOWN+3:
				button_inc_function(&down_function, &down_value);
				break;
			case JOYPAD_DOWNV+3:
				button_inc_value(&down_function, &down_value);
				break;
			case BUTTON_X+3:
				button_inc_function(&x_function, &x_value);
				break;
			case BUTTON_XV+3:
				button_inc_value(&x_function, &x_value);
				break;
			case BUTTON_O+3:
				button_inc_function(&o_function, &o_value);
				break;
			case BUTTON_OV+3:
				button_inc_value(&o_function, &o_value);
				break;
			case SHOULDER_LEFT+3:
				button_inc_function(&lshoulder_function, &lshoulder_value);
				break;
			case SHOULDER_LEFTV+3:
				button_inc_value(&lshoulder_function, &lshoulder_value);
				break;
			case SHOULDER_RIGHT+3:
				button_inc_function(&rshoulder_function, &rshoulder_value);
				break;
			case SHOULDER_RIGHTV+3:
				button_inc_value(&rshoulder_function, &rshoulder_value);
				break;
			case START_BUTTON+3:
				button_inc_function(&start_function, &start_value);
				break;
			case START_BUTTONV+3:
				button_inc_value(&start_function, &start_value);
				break;
			case SELECT_BUTTON+3:
				button_inc_function(&select_function, &select_value);
				break;
			case SELECT_BUTTONV+3:
				button_inc_value(&select_function, &select_value);
				break;
		}
	}

	if(button_state&O_PRESSED) return 1;

	if((button_state&X_PRESSED)&&(cursor_pos==(menu_y+EXIT))) return 1;

	return 0;
}

void draw_options(C64 *TheC64) {
	char *imagefile;
	char loadprg[255];
	int i;
	static int cursor_pos=3;
	static Prefs *prefs=NULL;
	int update_prefs=0;
	uint16 bg, fg;
	static int getfilename=0;
	static int xfiletype;
	static int set_buttons;
	static int menu_y=3;
	static int menu_x=5;
	static int is_moving=0;
	static int threshold=10;
	static int stepping=2;
#ifndef SPMP
	static int gpcpu_speed=133;
#endif
	static int load_snapshot=0;
	static int save_snapshot=0;

	enum { 
		D64, LOADSTAR, LOADER, PRG, RESET64,
		BLANK1,
		SOUND, BRIGHTNESS, CONTRAST, FRAMESKIP, LIMITSPEED,
		JOYSTICK, EMU1541CPU, 
		BLANK2,
		BUTTONS,
#ifndef SPMP
		CHATBOARD,
#endif
		BLANK3,
		LOAD_SNAP, SAVE_SNAP, 
		BLANK4,
#ifndef SPMP
		DEBUG, RESETGP,
#endif
		NUM_OPTIONS,
#ifndef SPMP
		CPU_SPEED,	/* WTF does this come after NUM_OPTIONS?? */
#endif
	};

	const char *option_txt[NUM_OPTIONS+5];
	option_txt[SOUND]=		"Sound                       ";
	option_txt[BRIGHTNESS]= 	"Brightness                  ";
	option_txt[CONTRAST]= 		"Contrast                    ";
	option_txt[FRAMESKIP]=		"Frameskip                   ";
	option_txt[LIMITSPEED]=		"Limit speed                 ";
	option_txt[JOYSTICK]=		"Joystick in port            ";
	option_txt[EMU1541CPU]=		"1541 CPU emulation          ";
	option_txt[D64]=		"Attach d64/t64/dir...       ";
	option_txt[LOADSTAR]=		"Load first program          ";
	option_txt[LOADER]=		"Load d64/dir browser        ";
	option_txt[PRG]=		"Load .prg file...           ";
	option_txt[SAVE_SNAP]=		"Save snapshot...            ";
	option_txt[LOAD_SNAP]=		"Load snapshot...            ";
#ifndef SPMP
	option_txt[CPU_SPEED]=		"gp32 CPU speed              ";
	option_txt[CHATBOARD]=		"Chatboard                   ";
#endif
	option_txt[BUTTONS]=		"Set buttons...              ";
	option_txt[RESET64]=		"Reset C=64                  ";
#ifndef SPMP
	option_txt[DEBUG]=		"Debug console               ";
	option_txt[RESETGP]=		"Restart GP32                ";
#endif
	option_txt[BLANK1]=		"                            ";
	option_txt[BLANK2]=		"                            ";
	option_txt[BLANK3]=		"                            ";
	option_txt[BLANK4]=		"                            ";
	option_txt[NUM_OPTIONS]=	"                            ";
	option_txt[NUM_OPTIONS+1]=	"                            ";
	option_txt[NUM_OPTIONS+2]=	"                            ";
	option_txt[NUM_OPTIONS+3]=	"                            ";
	option_txt[NUM_OPTIONS+4]=	NULL;

	char brightness_txt[3]="  ";
	char frameskip_txt[3]="  ";
	char contrast_txt[4]=" . ";

	uint8 menu_bg=14;
	uint8 menu_fg=6;
	uint8 menu_selbg=7;
	uint8 menu_selfg=6;

	if(load_snapshot||save_snapshot) {
		int sn=draw_snapshotui(TheC64, save_snapshot, prefs->DrivePath[0]);
		if(sn==-1) {
			// exit from snapshot ui
			load_snapshot=0;
			save_snapshot=0;
		} else if(!sn) {
			// snapshot ui stays open
			return;
		} else {
			// exit snapshot and options ui
			load_snapshot=0;
			save_snapshot=0;
			options_enabled=0;
			return;
		}
	}

	if(getfilename) {
		imagefile=filerequest(NULL);
		if(imagefile==NULL) {
			return;
		} else if (imagefile==(char *)-1) {
			getfilename=0;
		} else {
			if(xfiletype==0) {
				//printf("setting DrivePath[0] to -%s-\n", imagefile);
				strcpy(prefs->DrivePath[0], imagefile);
				update_prefs=1;
				cursor_pos=menu_y+LOADSTAR;
			} else if(xfiletype==1) {
				for(i=strlen(imagefile); i>0; i--) {
					if(imagefile[i]=='/') break;
				}
				strcpy(loadprg, "\rLOAD\"");
				strcat(loadprg, imagefile+i+1);
				strcat(loadprg, "\",8,1\rRUN\r");
				imagefile[i]='\0';
				strcpy(prefs->DrivePath[0], imagefile);
				for(i=0; loadprg[i]; i++) loadprg[i]=toupper(loadprg[i]);
				kbd_buf_feed(loadprg);
				update_prefs=1;
				options_enabled=0;
			}
			getfilename=0;
		}
	}

	if(set_buttons) {
		if(draw_buttonsui()) set_buttons=0;
		button_state&=~X_PRESSED;
		return;
	}

	if(prefs==NULL) prefs=new Prefs(ThePrefs);

	for(i=0; option_txt[i]; i++) {
		if(cursor_pos==(menu_y+i)) { 
			bg=menu_selbg;
			fg=menu_selfg; 
		} else { 
			bg=menu_bg;
			fg=menu_fg; 
		}
		text_out8((uint8 *)gp.pixels, option_txt[i], bg, fg, menu_x, menu_y+i, 0);
	}

	// sound state
	if(prefs->SIDType==SIDTYPE_NONE) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; 
	}
	text_out8((uint8 *)gp.pixels, "Off ",
			bg, fg, menu_x+20, menu_y+SOUND, 0);
	if(prefs->SIDType==SIDTYPE_DIGITAL) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; 
	}
	text_out8((uint8 *)gp.pixels, "On  ",
			bg, fg, menu_x+24, menu_y+SOUND, 0);
	bg=menu_bg;
	fg=menu_fg;

	// brightness
	brightness_txt[0]=(char)(0x30+((brightness%100)/10));
	brightness_txt[1]=(char)(0x30+((brightness%100)%10));
	text_out8((uint8 *)gp.pixels, brightness_txt,
			bg, fg, menu_x+20, menu_y+BRIGHTNESS, 0);
	// contrast
	contrast_txt[0]=(char)(0x30+((contrast%100)/10));
	contrast_txt[2]=(char)(0x30+((contrast%100)%10));
	text_out8((uint8 *)gp.pixels, contrast_txt,
			bg, fg, menu_x+20, menu_y+CONTRAST, 0);

	// frameskip
	frameskip_txt[0]=(char)(0x30+(((prefs->SkipFrames-1)%100)/10));
	frameskip_txt[1]=(char)(0x30+(((prefs->SkipFrames-1)%100)%10));
	text_out8((uint8 *)gp.pixels, frameskip_txt,
			bg, fg, menu_x+20, menu_y+FRAMESKIP, 0);

	// limit speed
	if(prefs->LimitSpeed==false) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; 
	}
	text_out8((uint8 *)gp.pixels, "Off ",
			bg, fg, menu_x+20, menu_y+LIMITSPEED, 0);
	if(prefs->LimitSpeed==true) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; 
	}
	text_out8((uint8 *)gp.pixels, "On  ",
			bg, fg, menu_x+24, menu_y+LIMITSPEED, 0);
	bg=menu_bg;
	fg=menu_fg;

	// current joyport
	if(current_joystick==0) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; }
	text_out8((uint8 *)gp.pixels, "1   ",
			bg, fg, menu_x+20, menu_y+JOYSTICK, 0);
	if(current_joystick==1) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; 
	}
	text_out8((uint8 *)gp.pixels, "2   ",
			bg, fg, menu_x+24, menu_y+JOYSTICK, 0);

	// 1541 cpu
	if(prefs->Emul1541Proc==false) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; }
	text_out8((uint8 *)gp.pixels, "Off ",
			bg, fg, menu_x+20, menu_y+EMU1541CPU, 0);
	if(prefs->Emul1541Proc==true) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; 
	}
	text_out8((uint8 *)gp.pixels, "On  ",
			bg, fg, menu_x+24, menu_y+EMU1541CPU, 0);

	// reset type
	if(prefs->FastReset==false) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; }
	text_out8((uint8 *)gp.pixels, "slow",
			bg, fg, menu_x+20, menu_y+RESET64, 0);
	if(prefs->FastReset==true) { 
		bg=menu_selbg;
		fg=menu_selfg; 
	} else { 
		bg=menu_bg;
		fg=menu_fg; 
	}
	text_out8((uint8 *)gp.pixels, "fast",
			bg, fg, menu_x+24, menu_y+RESET64, 0);

#ifndef SPMP
	// chatboard
	if(!chatboard_enabled) {
		bg=menu_selbg;
		fg=menu_selfg;
	} else {
		bg=menu_bg;
		fg=menu_fg; }
	text_out8((uint8 *)gp.pixels, "Off ",
			bg, fg, menu_x+20, menu_y+CHATBOARD, 0);
	if(chatboard_enabled) {
		bg=menu_selbg;
		fg=menu_selfg;
	} else {
		bg=menu_bg;
		fg=menu_fg; }
	text_out8((uint8 *)gp.pixels, "On  ",
			bg, fg, menu_x+24, menu_y+CHATBOARD, 0);
#endif

	i=menu_y+NUM_OPTIONS+1;
	// display attached file
	bg=menu_bg;
	fg=menu_fg;
	text_out8((uint8 *)gp.pixels, "drive 8 img: ", bg, fg, menu_x, i++, 0);
	text_out8((uint8 *)gp.pixels, prefs->DrivePath[0]+5, bg, fg, menu_x, i++, 0);
	text_out8((uint8 *)gp.pixels, "Press select to load", bg, fg, menu_x, i++, 0);

	if(button_state&X_PRESSED) {
		button_state&=~(X_PRESSED);
		if(cursor_pos==menu_y+LOADSTAR) {
			kbd_buf_feed("\rLOAD\":*\",8,1:\rRUN\r");
			options_enabled=0;
		}
#if 0
		if(cursor_pos==menu_y+LOADER) {
			load_prg(TheC64, c64loader, sizeof(c64loader));
			kbd_buf_feed("\rLOAD\"X\",8,1\rRUN\r");
			options_enabled=0;
		}
#endif
		if(cursor_pos==menu_y+D64) {
			getfilename=1;
			xfiletype=0;
		} else if(cursor_pos==menu_y+PRG) {
			getfilename=1;
			xfiletype=1;
		} else if(cursor_pos==menu_y+RESET64) {
			TheC64->Reset();
			options_enabled=0;
		} else if(cursor_pos==menu_y+SAVE_SNAP) {
			//remove("/smfs/gpmm/c64/frodo/snap1");
			//TheC64->SaveSnapshot("/smfs/gpmm/c64/frodo/snap1");
			//options_enabled=0;
			save_snapshot=1;
		} else if(cursor_pos==menu_y+LOAD_SNAP) {
			//TheC64->LoadSnapshot("/smfs/gpmm/c64/frodo/snap1");
			//options_enabled=0;
			load_snapshot=1;
		} else if(cursor_pos==menu_y+BUTTONS) {
			set_buttons=1;
#ifndef SPMP
		} else if(cursor_pos==menu_y+DEBUG) {
			switch_console();
#endif
		/*} else if(cursor_pos==10) {
			app_execute(prefs->DrivePath[0]);*/
#ifndef SPMP
		} else if(cursor_pos==menu_y+RESETGP) {
			gp32reboot();
#endif
		}
	} 
	
	if(button_state&UP_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_pos>menu_y) cursor_pos--;
		}
		is_moving++;
	} else if(button_state&DOWN_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_pos<menu_y+NUM_OPTIONS-1) cursor_pos++;
		}
		is_moving++;
	} else {
		is_moving=0;
	}

	if(button_state&LEFT_PRESSED) {
#ifndef SPMP
		if(cursor_pos==menu_y+CPU_SPEED) {
			if(gpcpu_speed==133) {
				gpcpu_speed=100;
				printf("setting cpu speed to 100mhz\n");
				//cpu_speed(102000000, (43<<12) | (1<<4) | 1, 2);

			}
		}
#endif
		if(cursor_pos==menu_y+SOUND) {
			prefs->SIDType=SIDTYPE_NONE; 
			update_prefs=1;
		}
		if(cursor_pos==menu_y+BRIGHTNESS) {
			if(brightness>0) {
				brightness--;
				//ui_set_palette();
			}
		}
		if(cursor_pos==menu_y+CONTRAST) {
			if(contrast>0) {
				contrast--;
				//ui_set_palette();
			}
		}
		if(cursor_pos==menu_y+FRAMESKIP) {
			if(prefs->SkipFrames>1) {
				prefs->SkipFrames--;
				update_prefs=1;
			}
		}
		if(cursor_pos==menu_y+LIMITSPEED) {
			prefs->LimitSpeed=false;
			update_prefs=1;
		}
		if(cursor_pos==menu_y+JOYSTICK) {
			current_joystick=0;
		}
		if(cursor_pos==menu_y+EMU1541CPU) {
			prefs->Emul1541Proc=false;
			update_prefs=1;
		}
		if(cursor_pos==menu_y+RESET64) {
			prefs->FastReset=false;
			update_prefs=1;
		}
#ifndef SPMP
		if(cursor_pos==menu_y+CHATBOARD) {
			disable_chatboard();
		}
#endif
	} else if(button_state&RIGHT_PRESSED) {
#ifndef SPMP
		if(cursor_pos==menu_y+CPU_SPEED) {
			if(gpcpu_speed==100) {
				gpcpu_speed=133;
				printf("setting cpu speed to 133mhz\n");
				//cpu_speed(133500000, (81<<12) | (2<<4) | 1, 2);
			}
		}
#endif
		if(cursor_pos==menu_y+SOUND) {
			prefs->SIDType=SIDTYPE_DIGITAL; 
			update_prefs=1;
		}
		if(cursor_pos==menu_y+BRIGHTNESS) {
			if(brightness<25) {
				brightness++;
				//ui_set_palette();
			}
		}
		if(cursor_pos==menu_y+CONTRAST) {
			if(contrast<100) {
				contrast++;
				//ui_set_palette();
			}
		}
		if(cursor_pos==menu_y+FRAMESKIP) {
			if(prefs->SkipFrames<100) {
				prefs->SkipFrames++;
				update_prefs=1;
			}
		}
		if(cursor_pos==menu_y+LIMITSPEED) {
			prefs->LimitSpeed=true;
			update_prefs=1;
		}
		if(cursor_pos==menu_y+JOYSTICK) {
			current_joystick=1;
		}
		if(cursor_pos==menu_y+EMU1541CPU) {
			prefs->Emul1541Proc=true;
			update_prefs=1;
		}
		if(cursor_pos==menu_y+RESET64) {
			prefs->FastReset=true;
			update_prefs=1;
		}
#if 0
		if(cursor_pos==menu_y+CHATBOARD) {
			enable_chatboard();
		}
#endif
	}

	if(update_prefs) {
		//printf("updating prefs\n");
		TheC64->NewPrefs(prefs);
		ThePrefs=*prefs;
		delete prefs;
		prefs=NULL;
		update_prefs=0;
	}
}

void draw_keyboard(C64 *TheC64) {
	const int keyb_width=27;
	const int keyb_height=8;
	static int keyb_start_x=14;
	static int keyb_start_y=21;
	static int shifted=0;
	static int rstopd=0;
	static int ctrled=0;
	static int rstred=0;
	static int gotkeypress;
	static int keyb_grabbed=0;
	int i;

	char keyb[keyb_height][keyb_width]={
		" =---=================*= ",
		"                         ",
		"    < 1234567890 del  F1 ",
		" ctrl QWERTYUIOP rstr F3 ",
		" r/st ASDFGHJKL; rtrn F5 ",
		" shft ZXCVBNM,./   .  F7 ",
		"  c    space      ...    ",
		"                         "
	};

	int keytable[] = {
		//x, y, key value
		6, 2, MATRIX(7,0), //1
		7, 2, MATRIX(7,3), //2
		8, 2, MATRIX(1,0), //3
		9, 2, MATRIX(1,3), //4
		10, 2, MATRIX(2,0), //5
		11, 2, MATRIX(2,3), //6
		12, 2, MATRIX(3,0), //7
		13, 2, MATRIX(3,3), //8
		14, 2, MATRIX(4,0), //9
		15, 2, MATRIX(4,3), //0

		6, 3, MATRIX(7,6), //q
		7, 3, MATRIX(1,1), //w
		8, 3, MATRIX(1,6), //e
		9, 3, MATRIX(2,1), //r
		10, 3, MATRIX(2,6), //t
		11, 3, MATRIX(3,1), //y
		12, 3, MATRIX(3,6), //u
		13, 3, MATRIX(4,1), //i
		14, 3, MATRIX(4,6), //o
		15, 3, MATRIX(5,1), //p

		6, 4, MATRIX(1,2), //a
		7, 4, MATRIX(1,5), //s
		8, 4, MATRIX(2,2), //d
		9, 4, MATRIX(2,5), //f
		10, 4, MATRIX(3,2), //g
		11, 4, MATRIX(3,5), //h
		12, 4, MATRIX(4,2), //j
		13, 4, MATRIX(4,5), //k
		14, 4, MATRIX(5,2), //l
		15, 4, MATRIX(6,2), //;

		6, 5, MATRIX(1,4), //z
		7, 5, MATRIX(2,7), //x
		8, 5, MATRIX(2,4), //c
		9, 5, MATRIX(3,7), //v
		10, 5, MATRIX(3,4), //b
		11, 5, MATRIX(4,7), //n
		12, 5, MATRIX(4,4), //m
		13, 5, MATRIX(5,7), //,
		14, 5, MATRIX(5,4), //.
		15, 5, MATRIX(6,7), ///

		2, 6, MATRIX(7,5), //c=

		//1, 4, MATRIX(7,7), //run/stop
		//2, 4, MATRIX(7,7), //
		//3, 4, MATRIX(7,7), //
		//4, 4, MATRIX(7,7), //

		4, 2, MATRIX(7,1), //<-

		7,6, MATRIX(7,4), // space
		8,6, MATRIX(7,4), // space
		9,6, MATRIX(7,4), // space
		10,6, MATRIX(7,4), // space
		11,6, MATRIX(7,4), // space

		17, 4, MATRIX(0,1), //return
		18, 4, MATRIX(0,1), //return
		19, 4, MATRIX(0,1), //return
		20, 4, MATRIX(0,1), //return

		19, 5, (MATRIX(0,7))|128, //crsr up
		19, 6, MATRIX(0,7), //crsr down
		18, 6, (MATRIX(0,2))|128, //crsr left
		20, 6, MATRIX(0,2), //crsr right

		17, 2, MATRIX(0,0), //delete
		18, 2, MATRIX(0,0), //delete
		19, 2, MATRIX(0,0), //delete

		22, 2, MATRIX(0,4), //f1
		23, 2, MATRIX(0,4), //f1
		22, 3, MATRIX(0,5), //f3
		23, 3, MATRIX(0,5), //f3
		22, 4, MATRIX(0,6), //f5
		23, 4, MATRIX(0,6), //f5
		22, 5, MATRIX(0,3), //f7
		23, 5, MATRIX(0,3), //f7

		NULL
	};


	text_out8((uint8 *)gp.pixels, keyb[0], 9, 1, keyb_start_x, keyb_start_y, 1);
	text_out8((uint8 *)gp.pixels, keyb[1], 9, 1, keyb_start_x, keyb_start_y+1, 1);
	text_out8((uint8 *)gp.pixels, keyb[2], 9, 1, keyb_start_x, keyb_start_y+2, 1);
	text_out8((uint8 *)gp.pixels, keyb[3], 9, 1, keyb_start_x, keyb_start_y+3, 1);
	text_out8((uint8 *)gp.pixels, keyb[4], 9, 1, keyb_start_x, keyb_start_y+4, 1);
	text_out8((uint8 *)gp.pixels, keyb[5], 9, 1, keyb_start_x, keyb_start_y+5, 1);
	text_out8((uint8 *)gp.pixels, keyb[6], 9, 1, keyb_start_x, keyb_start_y+6, 1);
	text_out8((uint8 *)gp.pixels, keyb[7], 9, 1, keyb_start_x, keyb_start_y+7, 1);

	// shift key
	if(shifted) {
		text_out8((uint8 *)gp.pixels, "shft", 7, 1, keyb_start_x+1, keyb_start_y+5, 0);
	}
	// run/stop key
	if(rstopd) {
		text_out8((uint8 *)gp.pixels, "r/st", 7, 1, keyb_start_x+1, keyb_start_y+4, 0);
	}
	// ctrl key
	if(ctrled) {
		text_out8((uint8 *)gp.pixels, "ctrl", 7, 1, keyb_start_x+1, keyb_start_y+3, 0);
	}
	// power led :)
	text_out8((uint8 *)gp.pixels, "*", 9, 2, keyb_start_x+22, keyb_start_y, 1);

	if(button_state&X_PRESSED) {
		// drag keyboard
		if(cursor_x>=keyb_start_x
				&&cursor_x<=(keyb_start_x+keyb_width)
				&&cursor_y==keyb_start_y
				&&cursor_x==cursor_x_last
				&&cursor_y==cursor_y_last
				) {
			keyb_grabbed=1;
		}
		/*
			if((cursor_x<cursor_x_last)
					&&(cursor_y==keyb_start_y)
					&& keyb_start_x>0) {
				keyb_start_x--;
			} else if((cursor_x>cursor_x_last)
					&&(cursor_y==keyb_start_y)
					&& keyb_start_x<30) {
				keyb_start_x++;
			}

			if((cursor_y<cursor_y_last)
					&&(cursor_y==keyb_start_y)
					&& keyb_start_y>0) {
				keyb_start_y--;
			} else if((cursor_y>cursor_y_last)
					&&(cursor_y==keyb_start_y)
					&& keyb_start_y<24) {
				keyb_start_y++;
			}*/
			/*if((button_state&LEFT_PRESSED)
					&&(cursor_y==keyb_start_y)
					&& keyb_start_x>0) {
				keyb_start_x--;
			} else if((button_state&RIGHT_PRESSED)
					&&(cursor_y==keyb_start_y)
					&& keyb_start_x<30) {
				keyb_start_x++;
			}

			if((button_state&UP_PRESSED)
					&&(cursor_y==keyb_start_y)
					&& keyb_start_y>0) {
				keyb_start_y--;
			} else if((button_state&DOWN_PRESSED)
					&&(cursor_y==keyb_start_y)
					&& keyb_start_y<24) {
				keyb_start_y++;
			}*/
		//}

		// key input
		for(i=0; keytable[i]; i+=3) {
			if((cursor_x==keyb_start_x+keytable[i])
					&& (cursor_y==keyb_start_y+keytable[i+1])) {
				vkey=keytable[i+2];
				if(shifted) vkey|=128;
				vkey_pressed=1;
				gotkeypress=1;
				//button_state &= ~A_PRESSED; // uli
				break;
			}
		}
		// shift key
		if(cursor_y==keyb_start_y+5
				&&cursor_x>=keyb_start_x+1
				&&cursor_x<=keyb_start_x+4) {
			shifted^=1;
			button_state&=~(X_PRESSED);
		}
		// ctrl key
		  else if(cursor_y==keyb_start_y+3
				&&cursor_x>=keyb_start_x+1
				&&cursor_x<=keyb_start_x+4) {
			if(ctrled==0) {
				ctrled=1;
				vkey=MATRIX(7,2);
				vkey_pressed=1;
				gotkeypress=1;
			} else {
				ctrled=0;
				vkey=MATRIX(7,2);
				vkey_pressed=0;
				gotkeypress=0;
				vkey_released=1;
			}

			button_state&=~(X_PRESSED);
		}
		// restore key
		  else if(cursor_y==keyb_start_y+3
				&&cursor_x>=keyb_start_x+17
				&&cursor_x<=keyb_start_x+20) {
			TheC64->NMI();
			button_state&=~(X_PRESSED);
		}
		// run/stop key
		  else if(cursor_y==keyb_start_y+4
				&&cursor_x>=keyb_start_x+1
				&&cursor_x<=keyb_start_x+4) {
			if(rstopd==0) {
				rstopd=1;
				vkey=MATRIX(7,7);
				if(shifted) vkey|=128;
				vkey_pressed=1;
				gotkeypress=1;
			} else {
				rstopd=0;
				vkey=MATRIX(7,7);
				if(shifted) vkey|=128;
				vkey_pressed=0;
				gotkeypress=0;
				vkey_released=1;
			}

			button_state&=~(X_PRESSED);
		}

	} else {
		keyb_grabbed=0;
		if(gotkeypress
				&& vkey!=MATRIX(7,7)
				&& vkey!=MATRIX(7,2)) {
			gotkeypress=0;
			vkey_pressed=0;
			vkey_released=1;
		}
	}

	if(keyb_grabbed) {
		if((cursor_x<cursor_x_last)
				&& keyb_start_x>0) {
			keyb_start_x--;
		} else if((cursor_x>cursor_x_last)
				&& keyb_start_x<30) {
			keyb_start_x++;
		}

		if((cursor_y<cursor_y_last)
				&& keyb_start_y>0) {
			keyb_start_y--;
		} else if((cursor_y>cursor_y_last)
				&& keyb_start_y<24) {
			keyb_start_y++;
		}
	}
}

void draw_status() {
	static char time[6]="00:00";
	static char speed[5]="000%";
	static char fps[3]="00";
	uint8 bg, fg;

	bg=14;
	fg=6;

	time[0]=(char)(0x30+(emu_minutes/10));
	time[1]=(char)(0x30+(emu_minutes%10));
	time[3]=(char)(0x30+(emu_seconds/10));
	time[4]=(char)(0x30+(emu_seconds%10));

	/*itoa_size(emu_minutes, time, 10, 3, '0');
	time[2]=':';
	itoa_size(emu_seconds, time+3, 10, 3, '0');*/
	text_out8((uint8 *)gp.pixels, time, bg, fg, 0, 29, 0);

	text_out8((uint8 *)gp.pixels, " speed ", bg, fg, 5, 29, 0);
	speed[0]=(char)(0x30+(emu_speed/100));
	speed[1]=(char)(0x30+((emu_speed%100)/10));
	speed[2]=(char)(0x30+((emu_speed%100)%10));
	text_out8((uint8 *)gp.pixels, speed, bg, fg, 12, 29, 0);

	text_out8((uint8 *)gp.pixels, " fps ", bg, fg, 16, 29, 0);
	fps[0]=(char)(0x30+((emu_fps%100)/10));
	fps[1]=(char)(0x30+((emu_fps%100)%10));
	text_out8((uint8 *)gp.pixels, fps, bg, fg, 21, 29, 0);
	text_out8((uint8 *)gp.pixels, "                 ", bg, fg, 23, 29, 0);

	if(drive8active) {
		bg=5;
		fg=6;
		text_out8((uint8 *)gp.pixels, "(8)", bg, fg, 30, 29, 0);
	}
}

void draw_cursor() {
	static int is_moving=0;
	int is_moving_x, is_moving_y;
	int threshold=10;
	int stepping=3;

	cursor_x_last=cursor_x;
	if(button_state&LEFT_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_x>0) {
				cursor_x--;
			}
		}
		is_moving_x=1;
	} else if(button_state&RIGHT_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_x<39) {
				cursor_x++;
			}
		}
		is_moving_x=1;
	} else is_moving_x=0;
	
	cursor_y_last=cursor_y;
	if(button_state&UP_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_y>0) {
				cursor_y--;
			}
		}
		is_moving_y=1;
	} else if(button_state&DOWN_PRESSED) {
		if(is_moving==0||((is_moving>threshold)&&(is_moving%stepping==0))) {
			if(cursor_y<29) {
				cursor_y++;
			}
		}
		is_moving_y=1;
	} else is_moving_y=0;

	if(is_moving_x||is_moving_y) {
		is_moving++;
	} else {
		is_moving=0;
	}
	
	text_out8((uint8 *)gp.pixels, "+", 0, 14, cursor_x, cursor_y, 1);
}

void draw_ui(C64 *TheC64) {
	int i;

	//printf("draw_ui %d/%d/%d\n", status_enabled, options_enabled, keyboard_enabled);
	if(status_enabled) draw_status();
	if(options_enabled) {
		if(!emu_paused) {
			TheC64->Pause();
			emu_paused=1;
		}
		draw_options(TheC64);
		return;
	} else {
		if(emu_paused) {
			TheC64->Resume();
			emu_paused=0;
		}
	}
	if(keyboard_enabled) {
		draw_keyboard(TheC64);
		draw_cursor();
	}
}

