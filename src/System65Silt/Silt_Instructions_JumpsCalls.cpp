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

// TODO: Surely there's a better way to do it...
int System65Silt::i_jmpind(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		// Strategy: get internal 16-bit ptr, zero-extend to 32-bit, add
		// m_Memory ptr for effective address, and use the address as a ptr to the
		// ultimate jump target (to load into DI).
		uint16_t offs = MAKEWORD(in[1],in[2]); // "internal" pointer to jump target
		*out++ = 0x66; // mov dx,<imm:offs>
		*out++ = 0xBA;
		*((uint16_t *)out) = offs; // <immediate:offs>
		out += 2;
		*out++ = 0x0F; // movzx edx,dx
		*out++ = 0xB7;
		*out++ = 0xD2;
		*out++ = 0x8D; // lea esi,[edx+<imm:m_Memory>]
		*out++ = 0xB2;
		*((uint32_t *)out) = *m_Memory; // <immediate:m_Memory>
		out += 4;
		*out++ = 0x66; // mov di, WORD PTR [esi]
		*out++ = 0x8B;
		*out++ = 0x3E;
	}

	stop = true;
	in += 3;
	count += 5;
	return 16;
}

int System65Silt::i_jsrabs(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		*out++ = 0xFF; // call dword ptr [siltasm_puship]
		*out++ = 0x15;
		*((void**)out) = siltasm_puship;
		out += 4;
		*out++ = 0x66; // 16-bit prefix
		*out++ = 0xBF; // mov di,imm
		*out++ = in[1]; // <imm>
		*out++ = in[2];
	}

	stop = true;
	in += 3;
	count += 6;
	return 10;
}

int System65Silt::i_rts(const uint8_t *&in, uint8_t *&out, int &count, bool &stop)
{
	if (out != NULL) {
		*out++ = 0xFF; // call dword ptr [siltasm_popip]
		*out++ = 0x15;
		*((void**)out) = siltasm_popip;
		out += 4;
		*out++ = 0x66; // inc di
		*out++ = 0x47;
	}

	stop = true;
	in += 1;
	count += 6;
	return 8;
}