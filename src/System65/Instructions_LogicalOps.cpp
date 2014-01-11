#include "System65/System65.hpp"

// Logical Operations
#define LOCAL_AND(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	a &= memory[addrmode]; \
	Helper_Set_ZN_Flags(a); \
	pc += isize
void SYSTEM65CORE System65::Insn_AND(void)
{
	switch(memory[pc]) {
	case 0x29: // immediate
		LOCAL_AND(2,2,Addr_IMM()); break;
	case 0x25: // zeropage
		LOCAL_AND(2,3,Addr_ZPG()); break;
	case 0x35: // zeropage,x
		LOCAL_AND(2,4,Addr_ZPX()); break;
	case 0x2d: // absolute
		LOCAL_AND(3,4,Addr_ABS()); break;
	case 0x3d: // absolute,x
		LOCAL_AND(3,4,Addr_ABX()); break;
	case 0x39: // absolute,y
		LOCAL_AND(3,4,Addr_ABY()); break;
	case 0x21: // (indirect,x)
		LOCAL_AND(2,6,Addr_INX()); break;
	case 0x31: // (indirect),y
		LOCAL_AND(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}
#undef LOCAL_AND

#define LOCAL_EOR(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	a ^= memory[addrmode]; \
	Helper_Set_ZN_Flags(a); \
	pc += isize
void SYSTEM65CORE System65::Insn_EOR(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
	switch(memory[pc]) {
	case 0x49: // immediate
		LOCAL_EOR(2,2,Addr_IMM()); break;
	case 0x45: // zeropage
		LOCAL_EOR(2,3,Addr_ZPG()); break;
	case 0x55: // zeropage,x
		LOCAL_EOR(2,4,Addr_ZPX()); break;
	case 0x4d: // absolute
		LOCAL_EOR(3,4,Addr_ABS()); break;
	case 0x5d: // absolute,x
		LOCAL_EOR(3,4,Addr_ABX()); break;
	case 0x59: // absolute,y
		LOCAL_EOR(3,4,Addr_ABY()); break;
	case 0x41: // (indirect,x)
		LOCAL_EOR(2,6,Addr_INX()); break;
	case 0x51: // (indirect),y
		LOCAL_EOR(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}
#undef LOCAL_EOR

#define LOCAL_ORA(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	a |= memory[addrmode]; \
	Helper_Set_ZN_Flags(a); \
	pc += isize
void SYSTEM65CORE System65::Insn_ORA(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
	switch(memory[pc]) {
	case 0x09: // immediate
		LOCAL_ORA(2,2,Addr_IMM()); break;
	case 0x05: // zeropage
		LOCAL_ORA(2,3,Addr_IMM()); break;
	case 0x15: // zeropage,x
		LOCAL_ORA(2,4,Addr_IMM()); break;
	case 0x0d: // absolute
		LOCAL_ORA(3,4,Addr_IMM()); break;
	case 0x1d: // absolute,x
		LOCAL_ORA(3,4,Addr_IMM()); break;
	case 0x19: // absolute,y
		LOCAL_ORA(3,4,Addr_IMM()); break;
	case 0x01: // (indirect,x)
		LOCAL_ORA(2,6,Addr_IMM()); break;
	case 0x11: // (indirect),y
		LOCAL_ORA(2,5,Addr_IMM()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}
#undef LOCAL_ORA

void SYSTEM65CORE System65::Insn_BIT(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
	uint8_t val;
	switch (memory[pc]) {
	case 0x24: // zeropage
		m_CycleCount += 3;
		val = memory[Addr_ZPG()];
	case 0x2c: // absolute
		m_CycleCount += 4;
		val = memory[Addr_ABS()];
	default:
		INSN_DECODE_ERROR(); return;
	}
	pc += 2;

	if (a & val)
		pf &= ~System65::PFLAG_Z;
	else
		pf |= System65::PFLAG_Z;

    // TODO: Faster/more concise bit setting
    if (val & 0b10000000)
		pf |= System65::PFLAG_N;
	else
		pf &= ~System65::PFLAG_N;

	if (val & 0b01000000)
		pf |= System65::PFLAG_V;
	else
		pf &= ~System65::PFLAG_V;
}
