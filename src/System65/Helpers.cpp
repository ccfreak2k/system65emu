#include "System65/System65.hpp"

// Helpers

void SYSTEM65CORE System65::Insn_Set_ZN_Flags(uint8_t reg)
{
	if (reg == 0)
		pf |= System65::PFLAG_Z;

	if (reg & 0b10000000)
		pf |= System65::PFLAG_N;
}
