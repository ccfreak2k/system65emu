#ifndef MAIN_HPP
#define MAIN_HPP

// Standard libs
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32

#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif // __cplusplus

// Class-related libs
#include <SFML/Graphics.hpp>

// Project libs
//#include "SDLContext.hpp"
#include "SFMLContext.hpp"
#include "System65.hpp"

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
//|                                                                                | 0010  00        brk         | 13
//|                                                                                | 0011  00        brk         | 14
//|                                                                                | 0012  00        brk         | 15
//|                                                                                | 0013  00        brk         | 16
//|                                                                                | 0014  00        brk         | 17
//|                                                                                | 0015  00        brk         | 18
//|                                                                                | 0016  00        brk         | 19
//|                                                                                | 0017  00        brk         | 20
//|                                                                                | 0018  00        brk         | 21
//|                                                                                | 0019  00        brk         | 22
//|                                                                                | 0020  00        brk         | 23
//|                                                                                | 0021  00        brk         | 24
//|                                                                                |                             | 25
//+--------------------------------------------------------------------------------+-----------------------------+ 26
//| Status line                                                                                                  | 27
//+--------------------------------------------------------------------------------------------------------------+ 28

// (112x29 characters)

// With an 8x16 font, this comes out to 896x464

// the memory viewer might be in a separate window, in which case the
// disassembly and CPU status might as well move too.

// The font file should be a texture atlas with 16x16 cells. Each cell should
// have an 8x16 character in it. The atlas texture size should thus be 144x256

#define TEXCHAR_WIDTH 8 // w/h of a character
#define TEXCHAR_HEIGHT 16
#define SCREEN_WIDTH 80 // Monitor size, in characters
#define SCREEN_HEIGHT 25
#define EMUSCREEN_WIDTH 112 // Emulator total size, also in characters
#define EMUSCREEN_HEIGHT 29

/** \fn int main(int argc, char **argv)
 * Main function for the emulator. This function initializes SDL and prepares
 * the emulated system for execution.
 *
 * \note Depending on whether this is the Win32 build, the function signature
 * currently changes. Consult the main.hpp file for the exact signatures.
 */

#ifndef WIN32
int main (int argc, char **argv);
#endif // WIN32

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
