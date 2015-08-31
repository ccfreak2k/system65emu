#include "System65/System65.hpp"

// Memory management

uint8_t SYSTEM65CORE System65::Memory_Read(uint16_t addr) {
	if (Memory_BoundsCheck(addr))
		return memory[addr];
	else
		return 0;
}

void SYSTEM65CORE System65::Memory_Write(uint16_t addr, uint8_t val) {
	if (Memory_BoundsCheck(addr))
		memory[addr] = val;
}

void SYSTEM65CORE System65::Memory_Write(uint16_t addr, uint16_t val) {
	Memory_Write(addr, (uint8_t)(val & 0xFF));
	Memory_Write((addr + 1), (uint8_t)((val >> 8) & 0xFF));
}

// TODO: Create callback system to see if the address is valid anywhere
bool SYSTEM65CORE System65::Memory_BoundsCheck(uint16_t addr) {
	if (addr > memorysize)
		return false;
	else
		return true;
}