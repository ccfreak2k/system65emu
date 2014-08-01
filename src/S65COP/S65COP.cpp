#include "S65COP.hpp"

S65COP::S65COP(uint8_t *base) :
	vx(),
	vy(),
	ix(0),
	iy(0),
	ip(0),
	scratch(new uint8_t[COP_SCRATCH_SIZE]),
	cmdbuf(base),
	cmdptr(0)
{
	// nothing else to do yet
}

S65COP::~S65COP()
{
	delete scratch;
}

void S65COP::Tick(void)
{
	// pre-exec
	if (cmdbus[COP_ADDR_EXECUTE] == 0x00)
		return;

	// post-exec
	if (cmdptr > 0x7F)
		cmdptr = 0x00;
}
