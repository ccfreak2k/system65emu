#include "System65/System65.hpp"

// Helpers

void SYSTEM65CORE System65::Helper_Set_ZN_Flags(uint8_t reg)
{
	if (reg == 0)
		pf |= System65::PFLAG_Z;

	if (reg & 0b10000000)
		pf |= System65::PFLAG_N;
}

void SYSTEM65CORE System65::Helper_PushByte(uint8_t val)
{
	memory[0x100+(--s)] = val;
}

void SYSTEM65CORE System65::Helper_PushWord(uint16_t val)
{
	Helper_PushByte(val >> 8);
	Helper_PushByte(val);
}

uint8_t SYSTEM65CORE System65::Helper_PopByte(void)
{
	return memory[0x100+(s++)];
}

uint16_t SYSTEM65CORE System65::Helper_PopWord(void)
{
	return (Helper_PopByte() + (Helper_PopByte() << 8));
}

void SYSTEM65CORE System65::Helper_SetBranch(bool branch)
{
	m_CycleCount += 2;
	if (branch) {
		m_CycleCount++;
		pc = (int8_t)memory[pc+1];
	} else {
		pc += 2;
	}
}
