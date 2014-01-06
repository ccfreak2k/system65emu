#include "System65/System65.hpp"

// Branches

void SYSTEM65CORE System65::Insn_BCC(void)
{
#if _DEBUG
	ASSERT_INSN(0x90);
#endif // _DEBUG
	Helper_SetBranch(!(pf && System65::PFLAG_C));
}

void SYSTEM65CORE System65::Insn_BCS(void)
{
#if _DEBUG
	ASSERT_INSN(0xb0);
#endif // _DEBUG
	Helper_SetBranch(pf & System65::PFLAG_C);
}

void SYSTEM65CORE System65::Insn_BEQ(void)
{
#if _DEBUG
	ASSERT_INSN(0xf0);
#endif // _DEBUG
	Helper_SetBranch(pf & System65::PFLAG_Z);
}

void SYSTEM65CORE System65::Insn_BMI(void)
{
#if _DEBUG
	ASSERT_INSN(0x30);
#endif // _DEBUG
	Helper_SetBranch(pf & System65::PFLAG_N);
}

void SYSTEM65CORE System65::Insn_BNE(void)
{
#if _DEBUG
	ASSERT_INSN(0xd0);
#endif // _DEBUG
	Helper_SetBranch(!(pf & System65::PFLAG_Z));
}

void SYSTEM65CORE System65::Insn_BPL(void)
{
#if _DEBUG
	ASSERT_INSN(0x10);
#endif // _DEBUG
	Helper_SetBranch(!(pf & System65::PFLAG_N));
}

void SYSTEM65CORE System65::Insn_BVC(void)
{
#if _DEBUG
	ASSERT_INSN(0x50);
#endif // _DEBUG
	Helper_SetBranch(!(pf & System65::PFLAG_V));
}

void SYSTEM65CORE System65::Insn_BVS(void)
{
#if _DEBUG
	ASSERT_INSN(0x70);
#endif // _DEBUG
	Helper_SetBranch(pf & System65::PFLAG_V);
}
