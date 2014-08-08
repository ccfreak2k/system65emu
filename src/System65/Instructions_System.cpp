#include "System65/System65.hpp"

// System operations

// TODO: Make sure PC is stored correctly after this is run
void SYSTEM65CORE System65::Insn_BRK(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x00);
#endif // _DEBUG
	// PC, FLAGS
	// BRK is handled very much like an interrupt on a real 6502, so we don't
	// bother doing anything here other than scheduling it.
	Helper_SetInterrupt(false, true);
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
	Helper_ClearFlag(System65::PFLAG_B);
	Helper_ClearFlag(System65::PFLAG_I);
	pc = Helper_PopWord();
	if (m_BreakFlagSet)
		pc += 2;
	m_BreakFlagSet = false;
}
