/*
 *  1541t64.cpp - 1541 emulation in .t64/LYNX file
 *
 *  Frodo (C) 1994-1997,2002-2003 Christian Bauer
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

/*
 * Notes:
 * ------
 *
 *  - If any file is opened, the contents of the file in the
 *    .t64 file are copied into a temporary file which is used
 *    for reading. This is done to insert the load address.
 *  - C64 LYNX archives are also handled by these routines
 *
 * Incompatibilities:
 * ------------------
 *
 *  - Only read accesses possible
 *  - No "raw" directory reading
 *  - No relative/sequential/user files
 *  - Only "I" and "UJ" commands implemented
 */

#include "sysdeps.h"

#include "1541t64.h"
#include "IEC.h"
#include "Prefs.h"


/*
 *  Constructor: Prepare emulation
 */

T64Drive::T64Drive(IEC *iec, char *filepath) : Drive(iec)
{
	the_file = NULL;
	file_info = NULL;

	Ready = false;
	strcpy(orig_t64_name, filepath);
	for (int i=0; i<16; i++)
		file[i] = NULL;

	// Open .t64 file
	open_close_t64_file(filepath);
	if (the_file != NULL) {
		Reset();
		Ready = true;
	}
}


/*
 *  Destructor
 */

T64Drive::~T64Drive()
{
	// Close .t64 file
	open_close_t64_file("");

	Ready = false;
}


/*
 *  Open/close the .t64/LYNX file
 */

void T64Drive::open_close_t64_file(char *t64name)
{
	uint8 buf[64];
	bool parsed_ok = false;

	// Close old .t64, if open
	if (the_file != NULL) {
		close_all_channels();
		fclose(the_file);
		the_file = NULL;
		delete[] file_info;
		file_info = NULL;
	}

	// Open new .t64 file
	if (t64name[0]) {
		if ((the_file = fopen(t64name, "rb")) != NULL) {

			// Check file ID
			fread(&buf, 64, 1, the_file);
			if (buf[0] == 0x43 && buf[1] == 0x36 && buf[2] == 0x34) {
				is_lynx = false;
				parsed_ok = parse_t64_file();
			} else if (buf[0x3c] == 0x4c && buf[0x3d] == 0x59 && buf[0x3e] == 0x4e && buf[0x3f] == 0x58) {
				is_lynx = true;
				parsed_ok = parse_lynx_file();
			}

			if (!parsed_ok) {
				fclose(the_file);
				the_file = NULL;
				delete[] file_info;
				file_info = NULL;
				return;
			}
		}
	}
}


/*
 *  Parse .t64 file and construct FileInfo array
 */

bool T64Drive::parse_t64_file(void)
{
	uint8 buf[32];
	uint8 *buf2;
	uint8 *p;
	int max, i, j;

	// Read header and get maximum number of files contained
	fseek(the_file, 32, SEEK_SET);
	fread(&buf, 32, 1, the_file);
	max = (buf[3] << 8) | buf[2];

	memcpy(dir_title, buf+8, 16);

	// Allocate buffer for file records and read them
	buf2 = new uint8[max*32];
	fread(buf2, 32, max, the_file);

	// Determine number of files contained
	for (i=0, num_files=0; i<max; i++)
		if (buf2[i*32] == 1)
			num_files++;

	if (!num_files)
		return false;

	// Construct file information array
	file_info = new FileInfo[num_files];
	for (i=0, j=0; i<max; i++)
		if (buf2[i*32] == 1) {
			memcpy(file_info[j].name, buf2+i*32+16, 16);

			// Strip trailing spaces
			file_info[j].name[16] = 0x20;
			p = file_info[j].name + 16;
			while (*p-- == 0x20) ;
			p[2] = 0;

			file_info[j].type = FTYPE_PRG;
			file_info[j].sa_lo = buf2[i*32+2];
			file_info[j].sa_hi = buf2[i*32+3];
			file_info[j].offset = (buf2[i*32+11] << 24) | (buf2[i*32+10] << 16) | (buf2[i*32+9] << 8) | buf2[i*32+8];
			file_info[j].length = ((buf2[i*32+5] << 8) | buf2[i*32+4]) - ((buf2[i*32+3] << 8) | buf2[i*32+2]);
			j++;
		}

	delete[] buf2;
	return true;
}


/*
 *  Parse LYNX file and construct FileInfo array
 */

bool T64Drive::parse_lynx_file(void)
{
	uint8 *p;
	int dir_blocks, cur_offset, num_blocks, last_block, i;
	char type_char;

	// Dummy directory title
	strcpy(dir_title, "LYNX ARCHIVE    ");

	// Read header and get number of directory blocks and files contained
	fseek(the_file, 0x60, SEEK_SET);
	fscanf(the_file, "%d", &dir_blocks);
	while (fgetc(the_file) != 0x0d)
		if (feof(the_file))
			return false;
	fscanf(the_file, "%d\015", &num_files);

	// Construct file information array
	file_info = new FileInfo[num_files];
	cur_offset = dir_blocks * 254;
	for (i=0; i<num_files; i++) {

		// Read file name
		fread(file_info[i].name, 16, 1, the_file);

		// Strip trailing shift-spaces
		file_info[i].name[16] = 0xa0;
		p = (uint8 *)file_info[i].name + 16;
		while (*p-- == 0xa0) ;
		p[2] = 0;

		// Read file length and type
		fscanf(the_file, "\015%d\015%c\015%d\015", &num_blocks, &type_char, &last_block);

		switch (type_char) {
			case 'S':
				file_info[i].type = FTYPE_SEQ;
				break;
			case 'U':
				file_info[i].type = FTYPE_USR;
				break;
			case 'R':
				file_info[i].type = FTYPE_REL;
				break;
			default:
				file_info[i].type = FTYPE_PRG;
				break;
		}
		file_info[i].sa_lo = 0;	// Only used for .t64 files
		file_info[i].sa_hi = 0;
		file_info[i].offset = cur_offset;
		file_info[i].length = (num_blocks-1) * 254 + last_block;

		cur_offset += num_blocks * 254;
	}

	return true;
}


/*
 *  Open channel
 */

uint8 T64Drive::Open(int channel, const uint8 *name, int name_len)
{
	set_error(ERR_OK);

	// Channel 15: Execute file name as command
	if (channel == 15) {
		execute_cmd(name, name_len);
		return ST_OK;
	}

	// Close previous file if still open
	if (file[channel]) {
		fclose(file[channel]);
		file[channel] = NULL;
	}

	if (name[0] == '#') {
		set_error(ERR_NOCHANNEL);
		return ST_OK;
	}

	if (the_file == NULL) {
		set_error(ERR_NOTREADY);
		return ST_OK;
	}

	if (name[0] == '$')
		return open_directory(channel, name + 1, name_len - 1);

	return open_file(channel, name, name_len);
}


/*
 *  Open file
 */

uint8 T64Drive::open_file(int channel, const uint8 *name, int name_len)
{
	uint8 plain_name[NAMEBUF_LENGTH];
	int plain_name_len;
	int mode = FMODE_READ;
	int type = FTYPE_PRG;
	int rec_len;
	parse_file_name(name, name_len, plain_name, plain_name_len, mode, type, rec_len);

	// Channel 0 is READ, channel 1 is WRITE
	if (channel == 0 || channel == 1) {
		mode = channel ? FMODE_WRITE : FMODE_READ;
		if (type == FTYPE_DEL)
			type = FTYPE_PRG;
	}

	bool writing = (mode == FMODE_WRITE || mode == FMODE_APPEND);

	// Wildcards are only allowed on reading
	if (writing && (strchr((const char *)plain_name, '*') || strchr((const char *)plain_name, '?'))) {
		set_error(ERR_SYNTAX33);
		return ST_OK;
	}

	// Allow only read accesses
	if (writing) {
		set_error(ERR_WRITEPROTECT);
		return ST_OK;
	}

	// Relative files are not supported
	if (type == FTYPE_REL) {
		set_error(ERR_UNIMPLEMENTED);
		return ST_OK;
	}

	// Find file
	int num;
	if (find_first_file(plain_name, plain_name_len, num)) {

		// Open temporary file
		if ((file[channel] = tmpfile()) != NULL) {

			// Write load address (.t64 only)
			if (!is_lynx) {
				fwrite(&file_info[num].sa_lo, 1, 1, file[channel]);
				fwrite(&file_info[num].sa_hi, 1, 1, file[channel]);
			}

			// Copy file contents from .t64 file to temp file
			uint8 *buf = new uint8[file_info[num].length];
			fseek(the_file, file_info[num].offset, SEEK_SET);
			fread(buf, file_info[num].length, 1, the_file);
			fwrite(buf, file_info[num].length, 1, file[channel]);
			rewind(file[channel]);
			delete[] buf;

			if (mode == FMODE_READ)	// Read and buffer first byte
				read_char[channel] = fgetc(file[channel]);
		}
	} else
		set_error(ERR_FILENOTFOUND);

	return ST_OK;
}


/*
 *  Find first file matching wildcard pattern
 */

// Return true if name 'n' matches pattern 'p'
static bool match(const uint8 *p, int p_len, const uint8 *n)
{
	while (p_len-- > 0) {
		if (*p == '*')	// Wildcard '*' matches all following characters
			return true;
		if ((*p != *n) && (*p != '?'))	// Wildcard '?' matches single character
			return false;
		p++; n++;
	}

	return *n == 0;
}

bool T64Drive::find_first_file(const uint8 *pattern, int pattern_len, int &num)
{
	for (int i=0; i<num_files; i++) {
		if (match(pattern, pattern_len, file_info[i].name)) {
			num = i;
			return true;
		}
	}
	return false;
}


/*
 *  Open directory, create temporary file
 */

uint8 T64Drive::open_directory(int channel, const uint8 *pattern, int pattern_len)
{
	// Special treatment for "$0"
	if (pattern[0] == '0' && pattern_len == 1) {
		pattern++;
		pattern_len--;
	}

	// Skip everything before the ':' in the pattern
	uint8 *t = (uint8 *)memchr(pattern, ':', pattern_len);
	if (t) {
		t++;
		pattern_len -= t - pattern;
		pattern = t;
	}

	// Create temporary file
	if ((file[channel] = tmpfile()) == NULL)
		return ST_OK;

	// Create directory title
	uint8 buf[] = "\001\004\001\001\0\0\022\042                \042 00 2A";
	for (int i=0; i<16 && dir_title[i]; i++)
		buf[i + 8] = dir_title[i];
	fwrite(buf, 1, 32, file[channel]);

	// Create and write one line for every directory entry
	for (int num=0; num<num_files; num++) {

		// Include only files matching the pattern
		if (pattern_len == 0 || match(pattern, pattern_len, file_info[num].name)) {

			// Clear line with spaces and terminate with null byte
			memset(buf, ' ', 31);
			buf[31] = 0;

			uint8 *p = buf;
			*p++ = 0x01;	// Dummy line link
			*p++ = 0x01;

			// Calculate size in blocks (254 bytes each)
			int n = (file_info[num].length + 254) / 254;
			*p++ = n & 0xff;
			*p++ = (n >> 8) & 0xff;

			p++;
			if (n < 10) p++;	// Less than 10: add one space
			if (n < 100) p++;	// Less than 100: add another space

			// Convert and insert file name
			uint8 str[NAMEBUF_LENGTH];
			memcpy(str, file_info[num].name, 17);
			*p++ = '\"';
			uint8 *q = p;
			for (int i=0; i<16 && str[i]; i++)
				*q++ = str[i];
			*q++ = '\"';
			p += 18;

			// File type
			switch (file_info[num].type) {
				case FTYPE_PRG:
					*p++ = 'P';
					*p++ = 'R';
					*p++ = 'G';
					break;
				case FTYPE_SEQ:
					*p++ = 'S';
					*p++ = 'E';
					*p++ = 'Q';
					break;
				case FTYPE_USR:
					*p++ = 'U';
					*p++ = 'S';
					*p++ = 'R';
					break;
				case FTYPE_REL:
					*p++ = 'R';
					*p++ = 'E';
					*p++ = 'L';
					break;
				default:
					*p++ = '?';
					*p++ = '?';
					*p++ = '?';
					break;
			}

			// Write line
			fwrite(buf, 1, 32, file[channel]);
		}
	}

	// Final line
	fwrite("\001\001\0\0BLOCKS FREE.             \0\0", 1, 32, file[channel]);

	// Rewind file for reading and read first byte
	rewind(file[channel]);
	read_char[channel] = fgetc(file[channel]);

	return ST_OK;
}


/*
 *  Close channel
 */

uint8 T64Drive::Close(int channel)
{
	if (channel == 15) {
		close_all_channels();
		return ST_OK;
	}

	if (file[channel]) {
		fclose(file[channel]);
		file[channel] = NULL;
	}

	return ST_OK;
}


/*
 *  Close all channels
 */

void T64Drive::close_all_channels(void)
{
	for (int i=0; i<15; i++)
		Close(i);

	cmd_len = 0;
}


/*
 *  Read from channel
 */

uint8 T64Drive::Read(int channel, uint8 &byte)
{
	int c;

	// Channel 15: Error channel
	if (channel == 15) {
		byte = *error_ptr++;

		if (byte != '\r')
			return ST_OK;
		else {	// End of message
			set_error(ERR_OK);
			return ST_EOF;
		}
	}

	if (!file[channel]) return ST_READ_TIMEOUT;

	// Get char from buffer and read next
	byte = read_char[channel];
	c = fgetc(file[channel]);
	if (c == EOF)
		return ST_EOF;
	else {
		read_char[channel] = c;
		return ST_OK;
	}
}


/*
 *  Write to channel
 */

uint8 T64Drive::Write(int channel, uint8 byte, bool eoi)
{
	// Channel 15: Collect chars and execute command on EOI
	if (channel == 15) {
		if (cmd_len >= 58)
			return ST_TIMEOUT;
		
		cmd_buf[cmd_len++] = byte;

		if (eoi) {
			execute_cmd(cmd_buf, cmd_len);
			cmd_len = 0;
		}
		return ST_OK;
	}

	if (!file[channel])
		set_error(ERR_FILENOTOPEN);
	else
		set_error(ERR_WRITEPROTECT);

	return ST_TIMEOUT;
}


/*
 *  Execute drive commands
 */

// RENAME:new=old
//        ^   ^
// new_file   old_file
void T64Drive::rename_cmd(const uint8 *new_file, int new_file_len, const uint8 *old_file, int old_file_len)
{
	// Check if destination file is already present
	int num;
	if (find_first_file(new_file, new_file_len, num)) {
		set_error(ERR_FILEEXISTS);
		return;
	}

	// Check if source file is present
	if (!find_first_file(old_file, old_file_len, num)) {
		set_error(ERR_FILENOTFOUND);
		return;
	}

	set_error(ERR_WRITEPROTECT);
}

// INITIALIZE
void T64Drive::initialize_cmd(void)
{
	close_all_channels();
}

// VALIDATE
void T64Drive::validate_cmd(void)
{
}


/*
 *  Reset drive
 */

void T64Drive::Reset(void)
{
	close_all_channels();
	cmd_len = 0;	
	set_error(ERR_STARTUP);
}
