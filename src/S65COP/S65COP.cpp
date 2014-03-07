#include "S65COP.hpp"

S65COP::S65COP(uint8_t *membus, uint8_t base) :
	vx(),
	vy(),
	ix(0),
	iy(0),
	scratch(new uint8_t[0x80]),
	sysmem(membus),
	cmdbuf(base),
	cmdptr(0)
{
	// nothing else to do yet
}

S65COP::~S65COP()
{
	delete scratch;
}
