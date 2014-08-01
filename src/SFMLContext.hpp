#ifndef SFMLCONTEXT_HPP
#define SFMLCONTEXT_HPP

// Class-related libs
#include <SFML/Graphics.hpp>

/** \file SFMLContext.hpp
 * Interface for the \ref SFMLContext class.
 */

/**
 * The class for anything SFML-related.
 *
 * This class serves as the interface between the emulator and SFML.
 */

class SFMLContext
{
	public:
		/** Initializes SFML and creates a render window
		 *
		 * \param[in] width Width of the render window
		 * \param[in] height Height of the render window
		 *
		 * \warning If SFML throws here, bad things might happen
		 */
		SFMLContext(int width, int height);

		/** Deletes the render window and SFML state, if any */
		~SFMLContext();

		/** Returns the SFML render window handle
		 *
		 * \return The SFML render window handle
		 *
		 * \note The handle is non-copyable.
		 */
		 //sf::RenderWindow GetRenderWindow() { return m_RenderWindow; };

		/** Draws the window contents */
		void Flip(void);

		sf::Window *m_Window; //!< SFML window handle
	protected:
	private:
		static const char *m_szWindowTitle; //!< Title to use for the render window
};

#endif // SFMLCONTEXT_HPP
