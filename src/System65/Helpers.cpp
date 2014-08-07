#include "System65/System65.hpp"

// Helpers

// TODO: Replace byte/word methods with overloads.

void SYSTEM65CORE System65::Helper_Set_ZN_Flags(uint8_t reg)
{
	if (reg == 0)
		pf |= System65::PFLAG_Z;
	else
		pf &= ~(System65::PFLAG_Z);

	if (reg & 0x80)
		pf |= System65::PFLAG_N;
	else
		pf &= ~(System65::PFLAG_N);
}

void SYSTEM65CORE System65::Helper_PushByte(uint8_t val)
{
	memory[m_StackBase+s] = val;
	s--;
}

void SYSTEM65CORE System65::Helper_PushWord(uint16_t val)
{
	Helper_PushByte(val >> 8);
	Helper_PushByte(val & 0xFF);
}

uint8_t SYSTEM65CORE System65::Helper_PopByte(void)
{
	s++;
	uint8_t val = memory[m_StackBase+s];
	return val;
}

uint16_t SYSTEM65CORE System65::Helper_PopWord(void)
{
	uint16_t val = Helper_PopByte();
	val += (Helper_PopByte() << 8);
	return val;
}

void SYSTEM65CORE System65::Helper_SetBranch(bool branch)
{
	m_CycleCount += 2;
	if (branch) {
		m_CycleCount++;
		// on a real 6502, pc is incremented after instruction fetching
		// and blah whatever so we have to add two bytes to the relative jump
		// destination for it to work right.
		pc += (int8_t)memory[pc+1];
	}
	pc += 2;
}

uint8_t SYSTEM65CORE System65::Helper_PeekByte(uint16_t addr)
{
	return memory[addr];
}

uint16_t SYSTEM65CORE System65::Helper_PeekWord(uint16_t addr)
{
	return memory[addr] + (memory[addr+1] << 8);
}

void SYSTEM65CORE System65::Helper_PokeByte(uint16_t addr, uint8_t val)
{
	memory[addr] = val;
}

void SYSTEM65CORE System65::Helper_PokeWord(uint16_t addr, uint16_t val)
{
	memory[addr] = val & 0xFF;
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

void SYSTEM65CORE System65::Helper_SetClear(System65::PFLAGS pflag, bool val)
{
	if (val)
		Helper_SetFlag(pflag);
	else
		Helper_ClearFlag(pflag);
}
