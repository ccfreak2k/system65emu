#include "System65/System65.hpp"

// Register Transfer

void SYSTEM65CORE System65::Insn_TAX(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0xaa);
#endif // _DEBUG
	INSN_R_TO_R(1,2,x,a);
	Helper_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_TAY(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0xa8);
#endif // _DEBUG
	INSN_R_TO_R(1,2,y,a);
	Helper_Set_ZN_Flags(y);
}

void SYSTEM65CORE System65::Insn_TXA(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x8a);
#endif // _DEBUG
	INSN_R_TO_R(1,2,a,x);
	Helper_Set_ZN_Flags(a);
}

void SYSTEM65CORE System65::Insn_TYA(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
#if _DEBUG
	ASSERT_INSN(0x98);
#endif // _DEBUG
	INSN_R_TO_R(1,2,a,y);
	Helper_Set_ZN_Flags(a);
}
