/*
 *  main_x.i - Main program, X specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 */

#define USBDEBUG

#include "Version.h"
#include <libgame.h>
#include <usbdbg.h>

extern int init_graphics(void);


/*
 *  Create application object and start it
 */
extern "C" void __call_exitprocs(int, void *);
extern emu_keymap_t keymap;
int my_exit(void)
{
	emuIfGraphCleanup();
	emuIfKeyCleanup(&keymap);
	emuIfSoundCleanup();
	fcloseall();
	libgame_chdir_game();
	unlink("frodo/_frodo.tmp");
	ThePrefs.Save("frodo/frodorc");
	__call_exitprocs(0, NULL);
	return NativeGE_gameExit();
}

int main(int argc, char **argv)
{
#ifdef USBDEBUG
	usbdbg_init();
	usbdbg_redirect_stdio(1);
#endif
	//usbdbg_blocking = 1;

	libgame_chdir_game();
	_ecos_mkdir("frodo", 0755);

	libgame_set_debug(0);
	//usbdbg_wait_for_plug();

	g_stEmuAPIs->exit = my_exit;
#ifndef USBDEBUG
	stdout = fopen("frodo/stdout.txt", "w");
	setbuf(stdout, NULL);
	stderr = fopen("frodo/stderr.txt", "w");
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

#ifndef USBDEBUG
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
#ifdef USBDEBUG
	usbdbg_puts("Frodo dtor\n");
#endif
}

/*
 *  Arguments processed, run emulation
 */
extern uint8 *font;
void Frodo::ReadyToRun(void)
{
	/* Our SPMP8k OS isn't so great with absolute paths... */
	strcpy(AppDirPath, "frodo");

	/* workaround for non-working global ctors */
	Prefs p;
	p.SkipFrames = 2;
	ThePrefs = p;

	ThePrefs.Load("frodo/frodorc");

	// Create and start C64
	TheC64 = new C64;
	printf("running frodo\n");
	load_rom_files();
	font = TheC64->Char;
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
