#ifndef MAIN_HPP
#define MAIN_HPP

// yaml_cpp needs this
#ifdef _MSC_VER
	#define _SCL_SECURE_NO_WARNINGS
#endif // _MSC_VER

// Standard libs
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#ifdef _MSC_VER
		#define _CRT_SECURE_NO_WARNINGS
	#endif
	#include <windows.h>
	#ifdef __MINGW32__
		#include <unistd.h>
	#else // probably msvc
		#include <stdint.h>
	#endif
#else
	#include <unistd.h>
#endif // WIN32

#include <atomic>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>

#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif // __cplusplus

// Class-related libs
#include <boost/program_options.hpp>
#include <SFML/Graphics.hpp>

// Project libs
//#include "SDLContext.hpp"
#include "SFMLContext.hpp"
#include "System65/System65.hpp"

/** \mainpage System65 Emulator
 *
 * Welcome to the System65 Emulator Doxygen pages. Here you can learn all about
 * the internals of the emulator. The emulator itself is mainly divided into
 * a few different modules and C++ classes. These are good starting points,
 * depending on what it is that you want to know:
 *
 * * System65 is the main class. It contains the full System65 emulation
 * context.
 *
 * * Within the System65 class are certain modules that have other potentially
 * useful pieces of information:
 *
 * * * The \ref module_registers "Registers" module describes each register and its use
 * within the 6502.
 *
 * * * The \ref module_addressmodes "Memory Addressing Modes" module describes the
 * various "address modes" used by each instruction.
 *
 * * * The \ref module_helpers "Execution Helper Functions" module describes
 * the various helper functions used within the emulator.
 *
 * * * The \ref module_instructions "CPU Instructions" module contains a list
 * and description of each supported CPU instruction.
 *
 * * S65COP is the System65 vector co-processor subsystem. As with the
 * System65 class, this too is fully self-contained, with the exception of
 * relying on System65 to work.
 *
 * * * The copinsnlist "Coprocessor Instruction Listing" module has a list
 * of each coprocessor instruction and their brief descriptions.
 *
 * * SFMLContext contains functionality necessary to interface with SFML
 * for accepting input from the user and displaying things on the screen. While
 * it is self-contained, it currently isn't complete enough to decouple the main
 * code from this class without some retooling.
 *
 * * SDLContext is much like SFMLContext, except it was used to interface
 * with SDL. It is currently unused, and is included mostly for historic value,
 * although an SDL port may be useful if we ever port to an exotic architecture
 * that SFML doesn't support.
 */

// The render window is currently planned to be rendered in "text mode," with
// all parts of the screen rendered as though it were a big console window. This
// may change later, since SDL can now effortlessly scale everything.

// If you'll excuse the wasted screen space, it might look like this:
//                                                                                                    111111111111
//          111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000001
//0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901
//+--------------------------------------------------------------------------------+-----------------------------+ 0
//|                                                                                | NV-BSIZC  A 00 X 00 PC 0000 | 1
//|                                                                                | 00100000  S 00 Y 00         | 2
//|                                                                                +-----------------------------+ 3
//|                                                                                |                             | 4
//|                                                                                | 0000  9E 0F E2  stz $E20F,x | 5
//|                                                                                | 0003  41 54     eor ($54,x) | 6
//|                                                                                | 0004  00        brk         | 7
//|                                                                                | 0005  00        brk         | 8
//|                                                                                | 0006  00        brk         | 9
//|                                                                                | 0007  00        brk         | 10
//|                                                                                | 0008  00        brk         | 11
//|                                                                                |>0009  00        brk         | 12
//|                                                                                | 000a  00        brk         | 13
//|                                                                                | 000b  00        brk         | 14
//|                                                                                | 000c  00        brk         | 15
//|                                                                                | 000d  00        brk         | 16
//|                                                                                | 000e  00        brk         | 17
//|                                                                                | 000f  00        brk         | 18
//|                                                                                | 0010  00        brk         | 19
//|                                                                                | 0011  00        brk         | 20
//|                                                                                | 0012  00        brk         | 21
//|                                                                                | 0013  00        brk         | 22
//|                                                                                | 0014  00        brk         | 23
//|                                                                                | 0015  00        brk         | 24
//|                                                                                |                             | 25
//+--------------------------------------------------------------------------------+-----------------------------+ 26
//| Status line                                                                                                  | 27
//+--------------------------------------------------------------------------------------------------------------+ 28

// (112x29 characters)

// With an 8x12 font, this comes out to 896x348

// the memory viewer might be in a separate window, in which case the
// disassembly and CPU status might as well move too.

// The font file should be a texture atlas with 16x16 cells. Each cell should
// have an 8x12 character in it. The atlas texture size should thus be 128x192

#define TEXCHAR_WIDTH 8 //!< Width of a single glyph on the texture sheet
#define TEXCHAR_HEIGHT 12 //!< Height of a single glyph on the texture sheet
#define TEXCHAR_CELLS 16 //!< Number of cells in either direction on the texture sheet
#define SCREEN_WIDTH 80 //!< Monitor width, in characters/columns
#define SCREEN_HEIGHT 25 //!< Monitor height, in characters/rows
#define EMUSCREEN_WIDTH 112 //!< Width of the entire emulator screen, in characters/columns
#define EMUSCREEN_HEIGHT 29 //!< Height of the entire emulator screen, in characters/rows

System65 sys(0x10000); //!< Object for the VM itself

std::atomic<bool> bStopExec = false; //!< Whether the VM should stop running; the thread will terminate

std::mutex mutMachineState; //!< Lock for the entire VM state

namespace po = boost::program_options;

/** \fn int main(int argc, char **argv)
 * Main function for the emulator. This function initializes SDL and prepares
 * the emulated system for execution.
 *
 * \note Depending on whether this is the Win32 build, the function signature
 * currently changes. Consult the main.hpp file for the exact signatures.
 */

//#ifndef WIN32
int main (int argc, char **argv);
//#endif // WIN32

/** Main thread for the System65 VM.
 *
 * This function manages the System65 execution state. It runs in a separate
 * thread and communicates with the main thread. The VM will run as long as
 * \ref mutMachineState is unlocked; if it is locked, the VM is stopped, and
 * this thread awaits commands.
 */
void SystemExec(void);

/** Draws the emulator screen frame.
 *
 * Draws the fancy ASCII border on the screen around the monitor, CPU status,
 * etc. Should only need to be called once at startup, but it may be called
 * whenever the border needs to be refreshed.
 *
 * \param[in] screenbuf Array of sf::Sprite representing the emulator screen
 */
void DrawScreenFrame(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT]);

/** Draws the emulator labels.
 *
 * Draws the labels that name various elements on the emulator screen.
 *
 * \param[in] screenbuf Array of sf::Sprite representing the emulator screen
 */
void DrawLabels(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT]);

/** Draws the CPU stats.
 *
 * Draws the status of the various parts of the system, including the current
 * register values, etc.
 *
 * \param[in] screenbuf Sprite array to draw into
 * \param[in] sys System65 object to get the status from
 */
void DrawStats(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT], System65 *sys);

/** Draws a string on the emulator screen.
 *
 * Draws a string starting at x,y, moving to the right as characters are
 * printed. If the edge of the screen is encountered, the remainder will be
 * printed on the next line.
 *
 * \param[in] screenbuf Sprite array to draw into
 * \param[in] str String to be drawn onto the screen
 * \param[in] x X coordinate to start printing at
 * \param[in] y Y coordinate to start printing at
 */
void DrawString(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT], const char *str, unsigned int x, unsigned int y);

/** Draws a character on the emulator screen.
 *
 * \note Position coords are relative to the entire emulator screen, not just
 * the monitor.
 *
 * \param[in] screenbuf Sprite array to draw into
 * \param[in] c ASCII character to draw
 * \param[in] x X position to draw at
 * \param[in] y Y position to draw at
 */
void DrawChar(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT], char c, unsigned int x, unsigned int y);

#endif // MAIN_HPP
