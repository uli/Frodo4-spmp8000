/*
 *  main_x.i - Main program, X specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 */

#include "Version.h"
#include <libgame.h>
#include <usbdbg.h>

extern int init_graphics(void);


/*
 *  Create application object and start it
 */

int main(int argc, char **argv)
{
	usbdbg_init();
	usbdbg_redirect_stdio(1);
	//usbdbg_blocking = 1;

	libgame_chdir_game();

	//usbdbg_wait_for_plug();

#if 0
	stdout = fopen("frodo_stdout.txt", "w");
	setbuf(stdout, NULL);
	stderr = fopen("frodo_stderr.txt", "w");
	setbuf(stderr, NULL);
#endif

	Frodo *the_app;

	printf("%s by Christian Bauer\n", VERSION_STRING);
	if (!init_graphics())
		return 0;
	fflush(stdout);

	the_app = new Frodo();
	the_app->ReadyToRun();
	delete the_app;

#if 0
	fclose(stdout);
	fclose(stderr);
#endif
	return 0;
}


/*
 *  Constructor: Initialize member variables
 */

Frodo::Frodo()
{
	TheC64 = NULL;
}

Frodo::~Frodo()
{
	usbdbg_puts("Frodo dtor\n");
}

/*
 *  Arguments processed, run emulation
 */

void Frodo::ReadyToRun(void)
{
	getcwd(AppDirPath, 256);

	/* workaround for non-working global ctors */
	Prefs p;
	p.SkipFrames = 2;
	ThePrefs = p;

	ThePrefs.Load("frodorc");

	// Create and start C64
	TheC64 = new C64;
	printf("running frodo\n");
	load_rom_files();
	printf("basic rom %p %0x\n", TheC64->Basic, TheC64->Basic[0]);
	printf("kernal rom %p %0x\n", TheC64->Kernal, TheC64->Kernal[0]);
	printf("char rom %p %0x\n", TheC64->Char, TheC64->Char[0]);
	TheC64->Run();
	printf("destroying frodo\n");
	delete TheC64;
}


bool IsDirectory(const char *path)
{
	return false;
}
