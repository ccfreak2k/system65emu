#include "System65Silt/System65Silt.hpp"

int System65Silt::i_nop(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL)
		*out++ = 0x90; // nop

	in += 1;
	count += 1;
	return 1;
}