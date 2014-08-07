#include "System65/System65.hpp"

// Arithemtic Operations

#define LOCAL_LOADVAL(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	val = memory[addrmode]; \
	pc += isize
void SYSTEM65CORE System65::Insn_ADC(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	// Status: coding complete, needs testing
	uint16_t val,nval;
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

	if (Helper_GetFlag(System65::PFLAG_D) != 0) {
		// BCD mode
#ifdef _DEBUG
		assert(false && "BCD not supported for ADC yet");
#endif // _DEBUG
		uint16_t vala = (a & 0x0f) + (val & 0x0f) + (Helper_GetFlag(System65::PFLAG_C) ? 1 : 0);
		if (vala > 9)
			vala = ((vala + 6) & 0x0f) + 16;

		uint16_t valb = (a & 0xf0) + (val & 0xf0) + vala;
		if (valb > 160)
			valb += 96;

		nval = (valb & 0xff);

		Helper_SetClearC((valb > 100));
		//HELPER_SETCLEARFLAG((valb > 100),System65::PFLAG_C);
		Helper_ClearFlag(System65::PFLAG_V);
	} else {
		// Non-BCD mode
		// Add w/ carry
		nval = a + val + (Helper_GetFlag(System65::PFLAG_C) ? 1 : 0);
		HELPER_SETCLEARFLAG((nval&0xFF00),System65::PFLAG_C); // TODO: Check me for neg values
		HELPER_SETCLEARFLAG((((a&0x80)==(val&0x80))&&((a&0x80)!=(nval&0x80))),System65::PFLAG_V);
		HELPER_SETCLEARFLAG(((nval&0xFF)==0),System65::PFLAG_Z);
		HELPER_SETCLEARFLAG((nval&0x80),System65::PFLAG_N);

		// Overflow occurs when the result is outside of the range -128 - 127.
		// Here are some examples:
		//    1 +  1      =    2, V = 0
		//    1 + -1      =    0, V = 0
		//  127 +  1      =  128, V = 1
		// -128 + -1      = -129, V = 1
		//   63 +  64 + 1 =  128, V = 1 (the +1 is the carry if set)
		// testadc.asm tests ADC and SBC to see if V handling is done correctly.
	}

	// write
	a = (uint8_t)nval;
}

void SYSTEM65CORE System65::Insn_SBC(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	// TODO: BCD mode subtract
	uint16_t val,nval;
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
	case 0xf1: // (indirect),y
		LOCAL_LOADVAL(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	if (Helper_GetFlag(System65::PFLAG_D) != 0) {
#ifdef _DEBUG
		assert(false && "BCD not supported for SBC yet");
#endif // _DEBUG
	} else {
		// subtract w/ carry
		nval = a - val - (Helper_GetFlag(System65::PFLAG_C) ? 0 : 1);
		HELPER_SETCLEARFLAG(!(nval&0xFF00),System65::PFLAG_C); // TODO: check me
		HELPER_SETCLEARFLAG((((a&0x80)!=(val&0x80))&&((a&0x80)!=(nval&0x80))),System65::PFLAG_V);
		HELPER_SETCLEARFLAG(((nval&0xFF)==0),System65::PFLAG_Z);
		HELPER_SETCLEARFLAG((nval&0x80),System65::PFLAG_N);

		// Overflow occurs when the result is outside of the range -128 - 127.
		// Here are some examples:
		//    0 -  1     = -1,   V = 0
		// -128 -  1     = -129, V = 1
		//  127 - -1     =  128, V = 1
		//  -64 - 64 - 1 = -129, V = 1 (the -1 is the carry if clear)
		// testadc.asm tests ADC and SBC to see if V handling is done correctly.
	}

	// write
	a = (uint8_t)nval;
}

void SYSTEM65CORE System65::Insn_CMP(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint8_t val;
	switch (memory[pc]) {
	case 0xc9: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xc5: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xd5: // zeropage,x
		LOCAL_LOADVAL(2,4,Addr_ZPX()); break;
	case 0xcd: // absolute
		LOCAL_LOADVAL(3,4,Addr_ABS()); break;
	case 0xdd: // absolute,x
		LOCAL_LOADVAL(3,4,Addr_ABX()); break;
	case 0xd9: // absolute,y
		LOCAL_LOADVAL(3,4,Addr_ABY()); break;
	case 0xc1: // (indirect,x)
		LOCAL_LOADVAL(2,6,Addr_INX()); break;
	case 0xd1: // (indirect),y
		LOCAL_LOADVAL(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	Helper_SetClearC(a >= val); // carry

	Helper_SetClearZ(a == val); // zero

	if ((a - val)&0x80) // negative
		Helper_SetFlag(System65::PFLAG_N);
	else
		Helper_ClearFlag(System65::PFLAG_N);
}

void SYSTEM65CORE System65::Insn_CPX(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint8_t val;
	switch(memory[pc]) {
	case 0xe0: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xe4: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xec: // absolute
		LOCAL_LOADVAL(3,4,Addr_ABS()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	if (x >= val) // carry
		pf |= System65::PFLAG_C;
	else
		pf &= ~(System65::PFLAG_C);

	if (x == val) // zero
		pf |= System65::PFLAG_Z;
	else
		pf &= ~(System65::PFLAG_Z);

	if ((x - val)&0x80) // negative
		pf |= System65::PFLAG_N;
	else
		pf &= ~(System65::PFLAG_N);
}

void SYSTEM65CORE System65::Insn_CPY(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
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
	else
		pf &= ~(System65::PFLAG_C);

	if (y == val) // zero
		pf |= System65::PFLAG_Z;
	else
		pf &= ~(System65::PFLAG_Z);

	if ((x - val)&0x80) // negative
		pf |= System65::PFLAG_N;
	else
		pf &= ~(System65::PFLAG_N);
}
#undef LOCAL_LOADVAL
