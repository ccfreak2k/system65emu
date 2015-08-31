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

void SYSTEM65CORE System65::Helper_Push(uint8_t val)
{
	//memory[m_StackBase+s] = val;
	Memory_Write(m_StackBase + s, val);
	s--;
}

void SYSTEM65CORE System65::Helper_Push(uint16_t val)
{
	Helper_Push((uint8_t)(val >> 8));
	Helper_Push((uint8_t)(val & 0xFF));
}

uint8_t SYSTEM65CORE System65::Helper_PopByte(void)
{
	s++;
	//uint8_t val = memory[m_StackBase+s];
	return Memory_Read(m_StackBase + s);
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
		//pc += (int8_t)memory[pc+1];
		pc += (int8_t)Memory_Read(pc + 1);
	}
	pc += 2;
}

// The following four methods are deprecated unless the need for them arises.
//uint8_t SYSTEM65CORE System65::Helper_PeekByte(uint16_t addr)
//{
//	return memory[addr];
//}

//uint16_t SYSTEM65CORE System65::Helper_PeekWord(uint16_t addr)
//{
//	return memory[addr] + (memory[addr+1] << 8);
//}

//void SYSTEM65CORE System65::Helper_Poke(uint16_t addr, uint8_t val)
//{
//	memory[addr] = val;
//}

//void SYSTEM65CORE System65::Helper_Poke(uint16_t addr, uint16_t val)
//{
//	memory[addr] = val & 0xFF;
//	memory[addr+1] = (val >> 8);
//}

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

void SYSTEM65CORE System65::Helper_SetInterrupt(bool nmi, bool sbrk)
{
	if ((!nmi) && (!sbrk)) { // F,F = IRQ
		m_BreakFlagSet = false;
		m_GenerateInterrupt = true;
		m_NMInterrupt = false;
		m_InterruptVector = 0xFFFE;
	} else if ((nmi) && (!sbrk)) { // T,F = NMI
		m_BreakFlagSet = false;
		m_GenerateInterrupt = true;
		m_NMInterrupt = true;
		m_InterruptVector = 0xFFFA;
	} else if ((!nmi) && (sbrk)){ // F,T = BRK
		m_BreakFlagSet = true;
		m_GenerateInterrupt = true;
		m_NMInterrupt = false;
		m_InterruptVector = 0xFFFE;
	} else // catchall
		assert(false && "impossible interrupt flag combination at Helper_SetInterrupt()");
}

bool SYSTEM65CORE System65::Helper_HandleInterrupt(void)
{
	assert(m_GenerateInterrupt && "Helper_HandleInterrupt() called when no interrupt scheduled");
	m_GenerateInterrupt = false;

	// First, can we service it?
	// If I is set, IRQ and BRK are skipped
	if (Helper_GetFlag(System65::PFLAG_I) && !m_NMInterrupt)
		return false;

	// From here either I is clear or this is an NMI.
	Helper_Push(m_BreakFlagSet ? (uint16_t)(pc+2) : pc);
	Helper_Push((uint8_t)(pf | (m_BreakFlagSet ? System65::PFLAG_B : 0x00)));
	Helper_SetFlag(System65::PFLAG_I);
	pc = Memory_ReadWord(m_InterruptVector);
	m_CycleCount += 7;
	return true;
}