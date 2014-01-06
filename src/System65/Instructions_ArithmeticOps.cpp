#include "System65/System65.hpp"

// Arithemtic Operations

#define LOCAL_LOADVAL(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	val = memory[addrmode]; \
	pc += isize
void SYSTEM65CORE System65::Insn_ADC(void)
{
	// TODO: BCD mode add
	uint16_t val;
	switch(memory[pc]) {
	case 0x69: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0x65: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0x75: // zeropage,x
		LOCAL_LOADVAL(2,4,Addr_ZPX()); break;
	case 0x6d: // absolute
		LOCAL_LOADVAL(3,4,Addr_ABS()); break;
	case 0x7d: // absolute,x
		LOCAL_LOADVAL(3,4,Addr_ABX()); break;
	case 0x79: // absolute,y
		LOCAL_LOADVAL(3,4,Addr_ABY()); break;
	case 0x61: // (indirect,x)
		LOCAL_LOADVAL(2,6,Addr_INX()); break;
	case 0x71: // (indirect),y
		LOCAL_LOADVAL(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	// Add w/ carry
	uint16_t nval = a + val + ((pf & System65::PFLAG_C) ? 1 : 0);

	// Set flags
	if (nval > 0xff) // carry
		pf |= System65::PFLAG_C;

	if (nval == 0) // zero
		pf |= System65::PFLAG_Z;

	if (((nval^a)&(nval^val)&0x80) > 0) // overflow
		pf |= System65::PFLAG_V;

	if (nval & 0b10000000) // negative
		pf |= System65::PFLAG_N;

	// write
	a = (uint8_t)nval;
}

void SYSTEM65CORE System65::Insn_SBC(void)
{
	// TODO: BCD mode subtract
	uint16_t val;
	switch(memory[pc]) {
	case 0xe9: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xe5: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xf5: // zeropage,x
		LOCAL_LOADVAL(2,4,Addr_ZPX()); break;
	case 0xed: // absolute
		LOCAL_LOADVAL(3,4,Addr_ABS()); break;
	case 0xfd: // absolute,x
		LOCAL_LOADVAL(3,4,Addr_ABX()); break;
	case 0xf9: // absolute,y
		LOCAL_LOADVAL(3,4,Addr_ABY()); break;
	case 0xe1: // (indirect,x)
		LOCAL_LOADVAL(2,6,Addr_INX()); break;
	case 0xd1: // (indirect),y
		LOCAL_LOADVAL(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	// subtract w/ carry
	uint16_t nval = a - val + ((pf & System65::PFLAG_C) ? 1 : 0) - 1;

	// Set flags
	if (!(nval&0x100)) // carry
		pf |= System65::PFLAG_C;

	if (nval == 0) // zero
		pf |= System65::PFLAG_Z;

	if (((nval^a)&(nval^-val)&0x80) > 0) // Overflow
		pf |= System65::PFLAG_V;

	if (nval & 0b10000000) // negative
		pf |= System65::PFLAG_N;

	// write
	a = (uint8_t)nval;
}

void SYSTEM65CORE System65::Insn_CMP(void)
{
	uint8_t val;
	switch (memory[pc]) {
	case 0xc9: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xc5: // zeropage
		LOCAL_LOADVAL(2,3,Addr_IMM()); break;
	case 0xd5: // zeropage,x
		LOCAL_LOADVAL(2,4,Addr_IMM()); break;
	case 0xcd: // absolute
		LOCAL_LOADVAL(3,4,Addr_IMM()); break;
	case 0xdd: // absolute,x
		LOCAL_LOADVAL(3,4,Addr_IMM()); break;
	case 0xd9: // absolute,y
		LOCAL_LOADVAL(3,4,Addr_IMM()); break;
	case 0xc1: // (indirect,x)
		LOCAL_LOADVAL(2,6,Addr_IMM()); break;
	case 0xd1: // (indirect),y
		LOCAL_LOADVAL(2,5,Addr_IMM()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	if (a >= val) // carry
		pf |= System65::PFLAG_C;

	if (a == val) // zero
		pf |= System65::PFLAG_Z;

	if ((a - val) < 0) // negative
		pf |= System65::PFLAG_N;
}

void SYSTEM65CORE System65::Insn_CPX(void)
{
	uint8_t val;
	switch(memory[pc]) {
	case 0xe0: // immediate
		LOCAL_LOADVAL(2,2,Addr_ABS()); break;
	case 0xe4: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xec: // absolute
		LOCAL_LOADVAL(3,4,Addr_ABS()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	if (x >= val) // carry
		pf |= System65::PFLAG_C;

	if (x == val) // zero
		pf |= System65::PFLAG_Z;

	if ((x - val) < 0) // negative
		pf |= System65::PFLAG_N;
}

void SYSTEM65CORE System65::Insn_CPY(void)
{
	uint8_t val;
	switch(memory[pc]) {
	case 0xc0: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xc4: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xcc: // absolute
		LOCAL_LOADVAL(2,3,Addr_ABS()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	if (y >= val) // carry
		pf |= System65::PFLAG_C;

	if (y == val) // zero
		pf |= System65::PFLAG_Z;

	if ((y - val) < 0) // negative
		pf |= System65::PFLAG_N;
}
#undef LOCAL_LOADVAL
