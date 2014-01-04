#include "SFMLContext.hpp"

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

const char *SFMLContext::m_szWindowTitle = "System65 Emulator";

SFMLContext::SFMLContext(int width, int height)
{
	m_Window(sf::VideoMode(width,height), m_szWindowTitle);
	m_Window.setFramerateLimit(60);
}

SFMLContext::~SFMLContext()
{
	//~m_Window();
}

void SFMLContext::Flip(void)
{
	m_Window.display();
}
