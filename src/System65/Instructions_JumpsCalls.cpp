#include "System65/System65.hpp"

// Jumps and Calls

// FIXME: Make sure this is setting PC correctly
void SYSTEM65CORE System65::Insn_JMP(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	switch (Memory_Read(pc)) {
	case 0x4c: // absolute
		m_CycleCount += 3;
		pc = Addr_ABS();
		break;
	case 0x6c: // indirect
		m_CycleCount += 5;
		pc = Addr_IND();
		break;
	default:
		INSN_DECODE_ERROR(); break;
	}
}

void SYSTEM65CORE System65::Insn_JSR(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x20);
#endif // _DEBUG

	m_CycleCount += 6;
	Helper_Push((uint16_t)(pc+2));
	//printf("[DEBUG] pc = 0x%.4X\n", pc);
	pc = Addr_ABS();
	//printf("[DEBUG] pc = 0x%.4X\n", pc);
}

void SYSTEM65CORE System65::Insn_RTS(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x60);
#endif // _DEBUG

	m_CycleCount += 6;
	//printf("[DEBUG] pc = 0x%.2X\n", pc);
	pc = Helper_PopWord()+1;
	//printf("[DEBUG] pc = 0x%.2X\n", pc);
}
