#ifndef SDLCONTEXT_HPP
#define SDLCONTEXT_HPP

// Standard libs
#include <exception>

// Class-related libs
#include <SDL2/SDL.h>

/** \file SDLContext.hpp
 * Interface for the \ref SDLContext class.
 */

/**
 * The class for anything SDL-related.
 *
 * This class serves as the interface for SDL and the emulator itself. It takes
 * care of the basic initialization and per-frame operations and serves as the
 * go-between for the emulator and SDL.
 *
 * To use this class, create a new SDLContext object...
 *
 * \code{.cpp}
 * SDLContext m_SDLContext = new SDLContext;
 * \endcode
 *
 * ...then create the new window:
 *
 * \code{.cpp}
 * m_SDLContext->InitVideo(640,480);
 * \endcode
 *
 * \todo Add more robust exception reporting
 *
 * \author ccfreak2k
 */

class SDLContext
{
	public:
		/** Initializes SDL
		 * \throw SDLInitException Thrown when SDL_Init() fails.
		 */
		SDLContext();

		/** Closes/Destroys the SDL system */
		~SDLContext();

		/** Initializes the window and renderer
		 * \param[in] width Width of the render window, default 640
		 * \param[in] height Height of the render window, default 480
		 */
		void InitVideo(int width = 640, int height = 480);

		/** Loads a font bitmap for rendering
		 * \param[in] filename Filename of the font bitmap to use, default "font.bmp"
		 */
		void LoadFont(const char *filename = "font.bmp")
	protected:
	private:
		SDL_Renderer *m_SDLRenderer; //!< SDL renderer handle
		SDL_Window   *m_SDLWindow;   //!< SDL window handle
		SDL_Surface  *m_FontSurface; //!< Font surface handle
		SDL_Texture  *m_FontTexture; //!< Font texture handle
		const char   *m_szWindowTitle = "System65 Emulator"; //!< Title for the emulator window
};

/**
 * Base class for SDLContext exceptions.
 *
 * This class is for exceptions that are thrown by \ref SDLContext. Other
 * SDLContext exceptions are based on this.
 *
 * This exception can be thrown as SDLContextUnknownException.
 *
 * \note This is a work in progress.
 */
class SDLContextException: public std::exception
{
	virtual const char* what() const throw() { return "Unknown SDLContext exception occurred"; }
} SDLContextUnknownException;



#endif // SDLCONTEXT_HPP
