/*
 *  1541t64.h - 1541 emulation in .t64/LYNX file
 *
 *  Frodo (C) 1994-1997,2002-2004 Christian Bauer
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

#ifndef _1541T64_H
#define _1541T64_H

#include "IEC.h"


// Information for file inside a .t64 file
typedef struct {
	uint8 name[17];		// File name, PETSCII
	uint8 type;			// File type
	uint8 sa_lo, sa_hi;	// Start address
	int offset;			// Offset of first byte in .t64 file
	int length;			// Length of file
} FileInfo;


class T64Drive : public Drive {
public:
	T64Drive(IEC *iec, char *filepath);
	virtual ~T64Drive();
	virtual uint8 Open(int channel, const uint8 *name, int name_len);
	virtual uint8 Close(int channel);
	virtual uint8 Read(int channel, uint8 &byte);
	virtual uint8 Write(int channel, uint8 byte, bool eoi);
	virtual void Reset(void);

private:
	void open_close_t64_file(char *t64name);
	bool parse_t64_file(void);
	bool parse_lynx_file(void);

	uint8 open_file(int channel, const uint8 *name, int name_len);
	uint8 open_directory(int channel, const uint8 *pattern, int pattern_len);
	bool find_first_file(const uint8 *pattern, int pattern_len, int &num);
	void close_all_channels(void);

	virtual void rename_cmd(const uint8 *new_file, int new_file_len, const uint8 *old_file, int old_file_len);
	virtual void initialize_cmd(void);
	virtual void validate_cmd(void);

	FILE *the_file;			// File pointer for .t64 file
	bool is_lynx;			// Flag: .t64 file is really a LYNX archive

	char orig_t64_name[256]; // Original path of .t64 file
	char dir_title[16];		// Directory title
	FILE *file[16];			// File pointers for each of the 16 channels (all temporary files)

	int num_files;			// Number of files in .t64 file and in file_info array
	FileInfo *file_info;	// Pointer to array of file information structs for each file

	uint8 read_char[16];	// Buffers for one-byte read-ahead
};

#endif
