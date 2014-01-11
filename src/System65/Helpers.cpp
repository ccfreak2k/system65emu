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
	memory[m_StackBase+(--s)] = val;
}

void SYSTEM65CORE System65::Helper_PushWord(uint16_t val)
{
	Helper_PushByte(val >> 8);
	Helper_PushByte(val);
}

uint8_t SYSTEM65CORE System65::Helper_PopByte(void)
{
	return memory[m_StackBase+(s++)];
}

uint16_t SYSTEM65CORE System65::Helper_PopWord(void)
{
	uint16_t val = Helper_PopByte();
	return val + (Helper_PopByte() << 8);
}

void SYSTEM65CORE System65::Helper_SetBranch(bool branch)
{
	m_CycleCount += 2;
	if (branch) {
		m_CycleCount++;
		pc += (int8_t)memory[pc+1];
	} else {
		pc += 2;
	}
}

uint8_t SYSTEM65CORE System65::Helper_PeekByte(uint16_t addr)
{
	return memory[addr];
}

uint16_t SYSTEM65CORE System65::Helper_PeekWord(uint16_t addr)
{
	return memory[addr+1] + (memory[addr] << 8);
}

void SYSTEM65CORE System65::Helper_PokeByte(uint16_t addr, uint8_t val)
{
	memory[addr] = val;
}

void SYSTEM65CORE System65::Helper_PokeWord(uint16_t addr, uint16_t val)
{
	memory[addr] = val;
	memory[addr+1] = (val >> 8);
}

void SYSTEM65CORE System65::Helper_SetFlag(System65::PFLAGS flag)
{
	pf |= flag;
}

void SYSTEM65CORE System65::Helper_ClearFlag(System65::PFLAGS flag)
{
	pf &= ~flag;
}

bool SYSTEM65CORE System65::Helper_GetFlag(System65::PFLAGS flag)
{
	return ((pf & flag) != 0);
}

void SYSTEM65CORE System65::Helper_SetClearC(bool val)
{
	if (val)
		Helper_SetFlag(System65::PFLAG_C);
	else
		Helper_ClearFlag(System65::PFLAG_C);
}

void SYSTEM65CORE System65::Helper_SetClearZ(bool val)
{
	if (val)
		Helper_SetFlag(System65::PFLAG_Z);
	else
		Helper_ClearFlag(System65::PFLAG_Z);
}
