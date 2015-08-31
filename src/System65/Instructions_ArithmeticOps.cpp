#include "System65/System65.hpp"

// Arithemtic Operations

#define LOCAL_LOADVAL(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	val = Memory_Read(addrmode); \
	pc += isize

void SYSTEM65CORE System65::Insn_ADC(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t val,nval;
	switch(Memory_Read(pc)) {
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
		uint16_t ba = ((a) >> 4)*10 + ((a) & 0x0F);
		uint16_t bval = (((val) >> 4)*10) + ((val) & 0x0F);

		nval = ba + bval + (Helper_GetFlag(System65::PFLAG_C) ? 1 : 0);
		Helper_SetClear(System65::PFLAG_C, (nval > 99));
		if (nval > 99)
			nval -= 100;
		a = ((((nval)/10) % 10) << 4) | ((nval) % 10);

		// V is valid in BCD (per the datasheets), however it's non-sensical
		// since it's only used for signed math. Nonetheless, its behavior in
		// BCD mode is covered here:
		// http://www.6502.org/tutorials/vflag.html#b
		// It's currently not touched here.
	} else {
		// Non-BCD mode
		// Add w/ carry
		nval = a + val + (Helper_GetFlag(System65::PFLAG_C) ? 1 : 0);
		Helper_SetClear(System65::PFLAG_C, (nval & 0xFF00) != 0); // TODO: Check me for neg values
		Helper_SetClear(System65::PFLAG_V, (((a & 0x80) == (val & 0x80)) && ((a & 0x80) != (nval & 0x80))) != 0);
		a = (uint8_t)nval;

		// Overflow occurs when the result is outside of the range -128 - 127.
		// Here are some examples:
		//    1 +  1      =    2, V = 0
		//    1 + -1      =    0, V = 0
		//  127 +  1      =  128, V = 1
		// -128 + -1      = -129, V = 1
		//   63 +  64 + 1 =  128, V = 1 (the +1 is the carry if set)
		// testadc.asm tests ADC and SBC to see if V handling is done correctly.
	}

	Helper_SetClear(System65::PFLAG_Z, (a & 0xFF) == 0);

	Helper_SetClear(System65::PFLAG_N, (a & 0x80) != 0);
}

void SYSTEM65CORE System65::Insn_SBC(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t val,nval;
	switch (Memory_Read(pc)) {
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
		uint16_t ba = ((a) >> 4)*10 + ((a) & 0x0F);
		uint16_t bval = (((val) >> 4)*10) + ((val) & 0x0F);

		nval = ba - bval - (Helper_GetFlag(System65::PFLAG_C) ? 0 : 1);
		Helper_SetClear(System65::PFLAG_C, (nval < 0x64)); // FIXME: Make sure this is correct
		if (nval > 0x63)
			nval += 100;
		a = ((((nval)/10) % 10) << 4) | ((nval) % 10);
	} else {
		// subtract w/ carry
		nval = a - val - (Helper_GetFlag(System65::PFLAG_C) ? 0 : 1);
		Helper_SetClear(System65::PFLAG_C, !(nval & 0xFF00)); // TODO: check me
		Helper_SetClear(System65::PFLAG_V, (((a & 0x80) != (val & 0x80)) && ((a & 0x80) != (nval & 0x80))) != 0);
		a = (uint8_t)nval;

		// Overflow occurs when the result is outside of the range -128 - 127.
		// Here are some examples:
		//    0 -  1     = -1,   V = 0
		// -128 -  1     = -129, V = 1
		//  127 - -1     =  128, V = 1
		//  -64 - 64 - 1 = -129, V = 1 (the -1 is the carry if clear)
		// testadc.asm tests ADC and SBC to see if V handling is done correctly.
	}

	Helper_SetClear(System65::PFLAG_Z, (a & 0xFF) == 0);

	Helper_SetClear(System65::PFLAG_N, (a & 0x80) != 0);
}

void SYSTEM65CORE System65::Insn_CMP(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint8_t val;
	switch (Memory_Read(pc)) {
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

	Helper_SetClear(System65::PFLAG_N, ((a - val) & 0x80) != 0); // negative
}

void SYSTEM65CORE System65::Insn_CPX(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint8_t val;
	switch (Memory_Read(pc)) {
	case 0xe0: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xe4: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xec: // absolute
		LOCAL_LOADVAL(3,4,Addr_ABS()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	Helper_SetClearC(x >= val); // carry

	Helper_SetClearZ(x == val); // zero

	Helper_SetClear(System65::PFLAG_N, ((x - val) & 0x80) != 0); // negative
}

void SYSTEM65CORE System65::Insn_CPY(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint8_t val;
	switch (Memory_Read(pc)) {
	case 0xc0: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xc4: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xcc: // absolute
		LOCAL_LOADVAL(3,3,Addr_ABS()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	Helper_SetClearC(y >= val); // carry

	Helper_SetClearZ(y == val); // zero

	Helper_SetClear(System65::PFLAG_N, ((y - val) & 0x80) != 0); // negative
}
#undef LOCAL_LOADVAL
