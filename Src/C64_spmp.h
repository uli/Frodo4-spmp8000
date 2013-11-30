/*
 *  C64_Amiga.i - Put the pieces together, Amiga specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 */

#include <libgame.h>



/*
 *  Constructor, system-dependent things
 */

void C64::c64_ctor1(void)
{
}

uint64_t start_time;
void C64::c64_ctor2(void)
{
	start_time = libgame_utime();
}


/*
 *  Destructor, system-dependent things
 */

void C64::c64_dtor(void)
{
}


/*
 *  Start emulation
 */

void C64::Run(void)
{
	// Reset chips
	TheCPU->Reset();
	TheSID->Reset();
	TheCIA1->Reset();
	TheCIA2->Reset();
	TheCPU1541->Reset();

	// Patch kernal IEC routines
	orig_kernal_1d84 = Kernal[0x1d84];
	orig_kernal_1d85 = Kernal[0x1d85];
	PatchKernal(ThePrefs.FastReset, ThePrefs.Emul1541Proc);

	// Start the CPU thread
	thread_running = true;
	quit_thyself = false;
	have_a_break = false;
	thread_func();
}


/*
 *  Stop emulation
 */

void C64::Quit(void)
{
#if 1
	// Ask the thread to quit itself if it is running
	if (thread_running) {
		quit_thyself = true;
		thread_running = false;
	}
#endif
}


/*
 *  Pause emulation
 */

void C64::Pause(void)
{
	TheSID->PauseSound();
}


/*
 *  Resume emulation
 */

void C64::Resume(void)
{
	TheSID->ResumeSound();
}


/*
 *  Vertical blank: Poll keyboard and joysticks, update window
 */


void C64::VBlank(bool draw_frame)
{
#if 1
	uint64_t end_time;
	long speed_index;
#endif

	// Poll keyboard
	TheDisplay->PollKeyboard(TheCIA1->KeyMatrix, TheCIA1->RevMatrix, &joykey);

	// Poll joysticks
	TheCIA1->Joystick1 = poll_joystick(0);
	TheCIA1->Joystick2 = poll_joystick(1);

	if (ThePrefs.JoystickSwap) {
		uint8 tmp = TheCIA1->Joystick1;
		TheCIA1->Joystick1 = TheCIA1->Joystick2;
		TheCIA1->Joystick2 = tmp;
	}

	// Joystick keyboard emulation
	if (TheDisplay->NumLock())
		TheCIA1->Joystick1 &= joykey;
	else
		TheCIA1->Joystick2 &= joykey;

	// Count TOD clocks
	TheCIA1->CountTOD();
	TheCIA2->CountTOD();

	// Update window if needed
	if (draw_frame) {
		//printf("drawing frame\n");
		TheDisplay->Update();

		// Calculate time between VBlanks, display speedometer
		end_time = libgame_utime();
		speed_index = 20000 * 100 * ThePrefs.SkipFrames / ((end_time - start_time) + 1);

		start_time = libgame_utime();

		TheDisplay->Speedometer(speed_index);
	}
}


/*
 *  Open/close joystick drivers given old and new state of
 *  joystick preferences
 */

void C64::open_close_joysticks(int oldjoy1, int oldjoy2, int newjoy1, int newjoy2)
{
}

#include "spmp_menu_input.h"
int current_joystick = 1;
extern int options_enabled;
extern int keyboard_enabled;
/*
 *  Poll joystick port, return CIA mask
 */

uint8 C64::poll_joystick(int port)
{
	uint8 j = 0xff;

	if(port!=current_joystick) return j;

	if(options_enabled||keyboard_enabled) return j;

	if(joy_state&LEFT_PRESSED) j&=0xfb;
	if(joy_state&RIGHT_PRESSED) j&=0xf7;
	if(joy_state&UP_PRESSED) j&=0xfe;
	if(joy_state&DOWN_PRESSED) j&=0xfd;
	if(joy_state&FIRE_PRESSED) j&=0xef;
	return j;
}


/*
 * The emulation's main loop
 */

void C64::thread_func(void)
{
	while (!quit_thyself) {
		//printf("main loop\n");
#ifdef FRODO_SC
		// The order of calls is important here
		if (TheVIC->EmulateCycle())
			TheSID->EmulateLine();
		TheCIA1->CheckIRQs();
		TheCIA2->CheckIRQs();
		TheCIA1->EmulateCycle();
		TheCIA2->EmulateCycle();
		TheCPU->EmulateCycle();

		if (ThePrefs.Emul1541Proc) {
			TheCPU1541->CountVIATimers(1);
			if (!TheCPU1541->Idle)
				TheCPU1541->EmulateCycle();
		}
		CycleCounter++;
#else
		// The order of calls is important here
		int cycles = TheVIC->EmulateLine();
		TheSID->EmulateLine();
#if !PRECISE_CIA_CYCLES
		TheCIA1->EmulateLine(ThePrefs.CIACycles);
		TheCIA2->EmulateLine(ThePrefs.CIACycles);
#endif

		if (ThePrefs.Emul1541Proc) {
			int cycles_1541 = ThePrefs.FloppyCycles;
			TheCPU1541->CountVIATimers(cycles_1541);

			if (!TheCPU1541->Idle) {
				// 1541 processor active, alternately execute
				//  6502 and 6510 instructions until both have
				//  used up their cycles
				while (cycles >= 0 || cycles_1541 >= 0)
					if (cycles > cycles_1541)
						cycles -= TheCPU->EmulateLine(1);
					else
						cycles_1541 -= TheCPU1541->EmulateLine(1);
			} else
				TheCPU->EmulateLine(cycles);
		} else
			// 1541 processor disabled, only emulate 6510
			TheCPU->EmulateLine(cycles);
#endif
	}
	//printf("thread_func() end\n");
}
