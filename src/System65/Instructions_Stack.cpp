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
	Helper_Push(a);
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
	uint8_t flags = pf;
	flags |= System65::PFLAG_B;
	Helper_Push(flags);
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
	HELPER_SETCLEARFLAG((a == 0),System65::PFLAG_Z);
	HELPER_SETCLEARFLAG((a>0x7F),System65::PFLAG_N);
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
	Helper_SetFlag(System65::PFLAG_R);
	if (m_BreakFlagSet)
		Helper_SetFlag(System65::PFLAG_B);
	else
		Helper_ClearFlag(System65::PFLAG_B);
	pc += 1;
}
