#include "System65/System65.hpp"

// Stack Operations

void SYSTEM65CORE System65::Insn_TSX(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0xba);
#endif // _DEBUG
	INSN_R_TO_R(1,2,x,s);
	Helper_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_TXS(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x9a);
#endif // _DEBUG
	INSN_R_TO_R(1,2,s,x);
}

void SYSTEM65CORE System65::Insn_PHA(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x48);
#endif // _DEBUG
	m_CycleCount += 3;
	Helper_PushByte(a);
	pc += 1;
}

void SYSTEM65CORE System65::Insn_PHP(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x08);
#endif // _DEBUG
	m_CycleCount += 3;
	Helper_PushByte(pf);
	pc += 1;
}

void SYSTEM65CORE System65::Insn_PLA(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x68);
#endif // _DEBUG
	m_CycleCount += 4;
	a = Helper_PopByte();
	pc += 1;
}

void SYSTEM65CORE System65::Insn_PLP(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x28);
#endif // _DEBUG
	m_CycleCount += 4;
	pf = Helper_PopByte();
	pf |= System65::PFLAG_R;
	pc += 1;
}
