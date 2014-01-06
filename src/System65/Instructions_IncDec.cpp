#include "System65/System65.hpp"

// Increment/Decrement

#define LOCAL_LOADADDR(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	addr = addrmode; \
	pc += isize
void SYSTEM65CORE System65::Insn_INC(void)
{
	uint16_t addr;
	switch(memory[pc]) {
	case 0xe6: // zeropage
		LOCAL_LOADADDR(2,5,Addr_ZPG()); break;
	case 0xf6: // zeropage,x
		LOCAL_LOADADDR(2,6,Addr_ZPX()); break;
	case 0xee: // absolute
		LOCAL_LOADADDR(3,6,Addr_ABS()); break;
	case 0xfe: // absolute,x
		LOCAL_LOADADDR(3,7,Addr_ABX()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	memory[addr]++;

	Insn_Set_ZN_Flags(memory[addr]);
}

#define LOCAL_INCR(isize,ccount,reg) \
	m_CycleCount += ccount; \
	reg++; \
	pc += isize
void SYSTEM65CORE System65::Insn_INX(void)
{
#if _DEBUG
	ASSERT_INSN(0xe8);
#endif // _DEBUG
	LOCAL_INCR(1,2,x);

	Insn_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_INY(void)
{
#if _DEBUG
	ASSERT_INSN(0xc8);
#endif // _DEBUG
	LOCAL_INCR(1,2,y);

	Insn_Set_ZN_Flags(y);
}
#undef LOCAL_INCR

void SYSTEM65CORE System65::Insn_DEC(void)
{
	uint16_t addr;
	switch(memory[pc]) {
	case 0xc6: // zeropage
		LOCAL_LOADADDR(2,5,Addr_ZPG()); break;
	case 0xd6: // zeropage,x
		LOCAL_LOADADDR(2,6,Addr_ZPX()); break;
	case 0xce: // absolute
		LOCAL_LOADADDR(3,6,Addr_ABS()); break;
	case 0xde: // absolute,x
		LOCAL_LOADADDR(3,7,Addr_ABX()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	memory[addr]--;

	Insn_Set_ZN_Flags(memory[addr]);
}
#undef LOCAL_LOADADDR

#define LOCAL_DECR(isize,ccount,reg) \
	m_CycleCount += ccount; \
	reg--; \
	pc += isize
void SYSTEM65CORE System65::Insn_DEX(void)
{
#if _DEBUG
	ASSERT_INSN(0xca);
#endif // _DEBUG
	LOCAL_DECR(1,2,x);

	Insn_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_DEY(void)
{
#if _DEBUG
	ASSERT_INSN(0x88);
#endif // _DEBUG
	LOCAL_DECR(1,2,y);

	Insn_Set_ZN_Flags(y);
}
#undef LOCAL_DECR
