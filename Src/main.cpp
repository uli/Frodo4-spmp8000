/*
 *  main.cpp - Main program
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"

#include "main.h"
#include "C64.h"
#include "Display.h"
#include "Prefs.h"
#include "SAM.h"


// Global variables
char AppDirPath[1024];	// Path of application directory


// ROM file names
#ifdef __riscos__
#define BASIC_ROM_FILE	"FrodoRsrc:Basic_ROM"
#define KERNAL_ROM_FILE	"FrodoRsrc:Kernal_ROM"
#define CHAR_ROM_FILE	"FrodoRsrc:Char_ROM"
#define FLOPPY_ROM_FILE	"FrodoRsrc:1541_ROM"
#else
#define BASIC_ROM_FILE	"Basic ROM"
#define KERNAL_ROM_FILE	"Kernal ROM"
#define CHAR_ROM_FILE	"Char ROM"
#define FLOPPY_ROM_FILE	"1541 ROM"
#endif


/*
 *  Load C64 ROM files
 */

#ifndef __PSXOS__
bool Frodo::load_rom_files(void)
{
	FILE *file;

	// Load Basic ROM
	if ((file = fopen(BASIC_ROM_FILE, "rb")) != NULL) {
		if (fread(TheC64->Basic, 1, 0x2000, file) != 0x2000) {
			ShowRequester("Can't read 'Basic ROM'.", "Quit");
			return false;
		}
		fclose(file);
	} else {
		ShowRequester("Can't find 'Basic ROM'.", "Quit");
		return false;
	}

	// Load Kernal ROM
	if ((file = fopen(KERNAL_ROM_FILE, "rb")) != NULL) {
		if (fread(TheC64->Kernal, 1, 0x2000, file) != 0x2000) {
			ShowRequester("Can't read 'Kernal ROM'.", "Quit");
			return false;
		}
		fclose(file);
	} else {
		ShowRequester("Can't find 'Kernal ROM'.", "Quit");
		return false;
	}

	// Load Char ROM
	if ((file = fopen(CHAR_ROM_FILE, "rb")) != NULL) {
		if (fread(TheC64->Char, 1, 0x1000, file) != 0x1000) {
			ShowRequester("Can't read 'Char ROM'.", "Quit");
			return false;
		}
		fclose(file);
	} else {
		ShowRequester("Can't find 'Char ROM'.", "Quit");
		return false;
	}

	// Load 1541 ROM
	if ((file = fopen(FLOPPY_ROM_FILE, "rb")) != NULL) {
		if (fread(TheC64->ROM1541, 1, 0x4000, file) != 0x4000) {
			ShowRequester("Can't read '1541 ROM'.", "Quit");
			return false;
		}
		fclose(file);
	} else {
		ShowRequester("Can't find '1541 ROM'.", "Quit");
		return false;
	}

	return true;
}
#endif


#ifdef __BEOS__
#include "main_Be.h"
#endif

#ifdef AMIGA
#include "main_Amiga.h"
#endif

#ifdef __unix
#include "main_x.h"
#endif

#ifdef __mac__
#include "main_mac.h"
#endif

#ifdef WIN32
#include "main_WIN32.h"
#endif

#ifdef __riscos__
#include "main_Acorn.h"
#endif
