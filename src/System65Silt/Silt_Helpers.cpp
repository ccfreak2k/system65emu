#include "System65Silt/System65Silt.hpp"

uint8_t System65Silt::Helper_PeekByte(uint16_t addr)
{
	return m_Memory[addr];
}

uint16_t System65Silt::Helper_PeekWord(uint16_t addr)
{
	return m_Memory[addr] + (m_Memory[addr+1] << 8);
}

void System65Silt::Helper_Poke(uint16_t addr, uint8_t val)
{
	m_Memory[addr] = val;
}

void System65Silt::Helper_Poke(uint16_t addr, uint16_t val)
{
	m_Memory[addr] = val & 0xFF;
	m_Memory[addr+1] = (val >> 8) & 0xFF;
}

void System65Silt::Helper_Push(uint8_t val)
{
	m_Memory[m_StackBase+m_Register.s] = val;
	m_Register.s--;
}

void System65Silt::Helper_Push(uint16_t val)
{
	Helper_Push((uint8_t)(val >> 8));
	Helper_Push((uint8_t)(val & 0xFF));
}

uint8_t System65Silt::Helper_PopByte(void)
{
	m_Register.s++;
	return m_Memory[m_StackBase+m_Register.s];
}

uint16_t System65Silt::Helper_PopWord(void)
{
	return (Helper_PopByte() & (Helper_PopByte() << 8));
}