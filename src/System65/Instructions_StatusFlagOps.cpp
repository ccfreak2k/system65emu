#include "System65/System65.hpp"

// Status flag operations

#define LOCAL_SET(flag) \
	m_CycleCount += 2; \
	pf |= flag; \
	pc += 1

#define LOCAL_CLEAR(flag) \
	m_CycleCount += 2; \
	pf &= ~flag; \
	pc += 1

void SYSTEM65CORE System65::Insn_CLC(void)
{
#if _DEBUG
	ASSERT_INSN(0x18);
#endif // _DEBUG
	LOCAL_CLEAR(System65::PFLAG_C);
}

void SYSTEM65CORE System65::Insn_CLD(void)
{
#if _DEBUG
	ASSERT_INSN(0xd8);
#endif // _DEBUG
	LOCAL_CLEAR(System65::PFLAG_D);
}

void SYSTEM65CORE System65::Insn_CLI(void)
{
#if _DEBUG
	ASSERT_INSN(0x58);
#endif // _DEBUG
	LOCAL_CLEAR(System65::PFLAG_I);
}

void SYSTEM65CORE System65::Insn_CLV(void)
{
#if _DEBUG
	ASSERT_INSN(0xb8);
#endif // _DEBUG
	LOCAL_CLEAR(System65::PFLAG_V);
}

void SYSTEM65CORE System65::Insn_SEC(void)
{
#if _DEBUG
	ASSERT_INSN(0x38);
#endif // _DEBUG
	LOCAL_SET(System65::PFLAG_C);
}

void SYSTEM65CORE System65::Insn_SED(void)
{
#if _DEBUG
	ASSERT_INSN(0xf8);
#endif // _DEBUG
	LOCAL_SET(System65::PFLAG_D);
}

void SYSTEM65CORE System65::Insn_SEI(void)
{
#if _DEBUG
	ASSERT_INSN(0x78);
#endif // _DEBUG
	LOCAL_SET(System65::PFLAG_I);
}
