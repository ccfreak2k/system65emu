#include "System65/System65.hpp"

// Register Transfer

void SYSTEM65CORE System65::Insn_TAX(void)
{
#if _DEBUG
	ASSERT_INSN(0xaa);
#endif // _DEBUG
	INSN_R_TO_R(1,2,x,a);
	Insn_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_TAY(void)
{
#if _DEBUG
	ASSERT_INSN(0xa8);
#endif // _DEBUG
	INSN_R_TO_R(1,2,y,a);
	Insn_Set_ZN_Flags(y);
}

void SYSTEM65CORE System65::Insn_TXA(void)
{
#if _DEBUG
	ASSERT_INSN(0x8a);
#endif // _DEBUG
	INSN_R_TO_R(1,2,a,x);
	Insn_Set_ZN_Flags(a);
}

void SYSTEM65CORE System65::Insn_TYA(void)
{
#if _DEBUG
	ASSERT_INSN(0x98);
#endif // _DEBUG
	INSN_R_TO_R(1,2,a,y);
	Insn_Set_ZN_Flags(a);
}

// Stack Operations

void SYSTEM65CORE System65::Insn_TSX(void)
{
#if _DEBUG
	ASSERT_INSN(0xba);
#endif // _DEBUG
	INSN_R_TO_R(1,2,x,s);
	Insn_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_TXS(void)
{
#if _DEBUG
	ASSERT_INSN(0x9a);
#endif // _DEBUG
	INSN_R_TO_R(1,2,s,x);
}

void SYSTEM65CORE System65::Insn_PHA(void)
{
#if _DEBUG
	ASSERT_INSN(0x48);
#endif // _DEBUG
	m_CycleCount += 3;
	memory[--s] = a;
	pc += 1;
}

void SYSTEM65CORE System65::Insn_PHP(void)
{
#if _DEBUG
	ASSERT_INSN(0x08);
#endif // _DEBUG
	m_CycleCount += 3;
	memory[--s] = pf;
	pc += 1;
}

void SYSTEM65CORE System65::Insn_PLA(void)
{
#if _DEBUG
	ASSERT_INSN(0x68);
#endif // _DEBUG
	m_CycleCount += 4;
	a = memory[s++];
	Insn_Set_ZN_Flags(a);
	pc += 1;
}

void SYSTEM65CORE System65::Insn_PLP(void)
{
#if _DEBUG
	ASSERT_INSN(0x28);
#endif // _DEBUG
	m_CycleCount += 4;
	pf = memory[s++]|System65::PFLAG_R;
	pc += 1;
}
