#include "System65Silt/System65Silt.hpp"

int System65Silt::i_ldaimm(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		*out++ = 0xB0;  // mov al, imm
		*out++ = in[1]; // <imm>
	}

	in += 2;
	count += 2;
	return 2;
}

int System65Silt::i_ldazpg(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	return 0;
}

int System65Silt::i_ldazpx(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	return 0;
}

int System65Silt::i_ldaabs(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	return 0;
}

int System65Silt::i_ldaabx(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		// zero-extend X register offset (in bl) to edx
		*out++ = 0x0F; // movzx edx,bl
		*out++ = 0xB6;
		*out++ = 0xD3;

		// decode specified absolute address
		uint16_t offs = MAKEWORD(in[1], in[2]);

		// calculate true address in emulated memory
		uint32_t addr = (uint32_t)(m_Memory + offs);

		// load address plus offset (placed in edx above) into esi
		*out++ = 0x8D; // lea esi,[edx+imm]
		*out++ = 0xB2;
		*((uint32_t *)out) = addr; // <imm>
		out += 4;

		// load accumulator (in al) from memory pointed at by esi
		*out++ = 0x8A; // mov al,[esi]
		*out++ = 0x06;
	}

	in += 3;
	count += 5;
	return 11;
}

int System65Silt::i_ldaaby(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	return 0;
}

int System65Silt::i_ldainx(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	return 0;
}

int System65Silt::i_ldainy(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	return 0;
}

//------------------------------------------------------------------------------

int System65Silt::i_ldximm(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		*out++ = 0xB3; // mov bl,imm
		*out++ = in[1]; // <imm>
	}

	in += 2;
	count += 2;
	return 2;
}

//------------------------------------------------------------------------------

int System65Silt::i_ldyimm(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		*out++ = 0xB7; // mov bh,imm
		*out++ = in[1]; // <imm>
	}

	return 0;
}

//------------------------------------------------------------------------------

int System65Silt::i_staabs(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		// decode specified absolute address
		uint16_t offs = MAKEWORD(in[1],in[2]);

		// calculate true address in emulated memory
		uint32_t addr = (uint32_t)(m_Memory + offs);

		// store accumulator (in al) to memory pointed by esi
		*out++ = 0x88; // mov [esi],al
		*out++ = 0x06;
	}

	return 0;
}

int System65Silt::i_staabx(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		// zero-extend X register offset (in bl) to edx
		*out++ = 0x0F; // movezx edx, bl
		*out++ = 0xB6;
		*out++ = 0xD3;

		// decode specified absolute address
		uint16_t offs = MAKEWORD(in[1], in[2]);

		// calculate true address in emulated memory
		uint32_t addr = (uint32_t)(m_Memory + offs);

		// load address plus offset (placed in edx above) into esi
		*out++ = 0x8D; // lea esi,[edx+imm]
		*out++ = 0xB2;
		*((uint32_t *)out) = addr; // <imm>
		*out += 4;

		// store accumulator (in al) to memory pointed by esi
		*out++ = 0x88; // mov [esi],al
		*out++ = 0x06;
	}

	in += 3;
	count += 5;
	return 11;
}