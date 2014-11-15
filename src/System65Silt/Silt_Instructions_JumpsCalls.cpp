#include "System65Silt/System65Silt.hpp"

int System65Silt::i_jmpabs(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		*out++ = 0x66; // 16-bit prefix
		*out++ = 0xBF; // mov di,imm
		*out++ = in[1]; // <imm>
		*out++ = in[2];
	}

	stop = true;
	in += 3;
	count += 3;
	return 4;
}

int System65Silt::i_jmpind(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		
	}
}