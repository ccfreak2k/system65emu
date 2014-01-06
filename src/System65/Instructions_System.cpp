#include "System65/System65.hpp"

// System operations

void SYSTEM65CORE System65::Insn_BRK(void)
{
#if _DEBUG
	ASSERT_INSN(0x00);
#endif // _DEBUG
	// PC, FLAGS
	m_CycleCount += 7;
	Helper_PushWord(pc);
	Helper_PushByte(pf);
	pc = Helper_PeekWord(0xFFFE);
}

void SYSTEM65CORE System65::Insn_NOP(void)
{
#if _DEBUG
	ASSERT_INSN(0xea);
#endif // _DEBUG
	m_CycleCount += 2;
	pc += 1;
}

void SYSTEM65CORE System65::Insn_RTI(void)
{
#if _DEBUG
	ASSERT_INSN(0x40);
#endif // _DEBUG
	m_CycleCount += 6;
	pf = Helper_PopByte();
	Helper_SetFlag(System65::PFLAG_R);
	pc = Helper_PopWord();
}
