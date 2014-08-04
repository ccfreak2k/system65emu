#include "System65/System65.hpp"

// System operations

void SYSTEM65CORE System65::Insn_BRK(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x00);
#endif // _DEBUG
	// PC, FLAGS
	m_CycleCount += 7;
	Helper_PushWord(pc);
	Helper_PushByte(pf);
	Helper_SetFlag(System65::PFLAG_B);
	pc = Helper_PeekWord(0xFFFE);
}

void SYSTEM65CORE System65::Insn_NOP(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0xea);
#endif // _DEBUG
	m_CycleCount += 2;
	pc += 1;
}

void SYSTEM65CORE System65::Insn_RTI(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x40);
#endif // _DEBUG
	m_CycleCount += 6;
	pf = Helper_PopByte();
	// Food for thought: The user can arbitrarily set flags while they're in
	// memory. The B flag is only set when servicing a software (BRK-triggered)
	// interrupt. With that in mind, is it proper to clear the B flag here, or
	// does it need to keep the value from memory?
	Helper_SetFlag(System65::PFLAG_R);
	Helper_ClearFlag(System65::PFLAG_B);
	pc = Helper_PopWord();
}
