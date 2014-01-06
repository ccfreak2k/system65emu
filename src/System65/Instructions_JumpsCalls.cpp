#include "System65/System65.hpp"

// Jumps and Calls

// FIXME: Make sure this is setting PC correctly
void SYSTEM65CORE System65::Insn_JMP(void)
{
	switch (memory[pc]) {
	case 0x4c: // absolute
		m_CycleCount += 3;
		pc = memory[Addr_ABS()];
		break;
	case 0x6c: // indirect
		m_CycleCount += 5;
		pc = memory[Addr_IND()];
		break;
	default:
		INSN_DECODE_ERROR(); break;
	}
}

void SYSTEM65CORE System65::Insn_JSR(void)
{
#if _DEBUG
	ASSERT_INSN(0x20);
#endif // _DEBUG

	m_CycleCount += 6;
	Helper_PushWord(pc+1);
	pc = memory[Addr_ABS()];
}

void SYSTEM65CORE System65::Insn_RTS(void)
{
#if _DEBUG
	ASSERT_INSN(0x60);
#endif // _DEBUG

	m_CycleCount += 6;
	pc = Helper_PopByte()+1;
}
