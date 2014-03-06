#include "System65/System65.hpp"

// Address modes

// Each of these methods returns a 16-bit index (i.e. a pointer) into emulator
// memory for the operand of the instruction in question.

// The Accumulator (Addr_ACC) and Relative (Addr_REL) modes do not use a memory
// index, so calling the respective methods is an error. They are here simply
// for completion and as placeholders for possible future refactoring.

// Accumulator
uint8_t SYSTEM65CORE System65::Addr_ACC(void){ assert(false); }

// Absolute
uint16_t SYSTEM65CORE System65::Addr_ABS(void)
{
	// FIXME: make sure types match and don't corrupt the retval
	// Further, the 6502 is little-endian so make sure the value is retrieved
	// correctly.
	return (uint16_t)((uint16_t)(memory[pc+2] << 8) + (uint16_t)memory[pc+1]);
}

// Absolute,X
uint16_t SYSTEM65CORE System65::Addr_ABX(void)
{
	return (uint16_t)((uint16_t)(memory[pc+2] << 8) + (uint16_t)memory[pc+1]) + x;
}

// Absolute,y
uint16_t SYSTEM65CORE System65::Addr_ABY(void)
{
	return (uint16_t)((uint16_t)(memory[pc+2] << 8) + (uint16_t)memory[pc+1]) + y;
}

// Immediate
uint16_t SYSTEM65CORE System65::Addr_IMM(void)
{
	return pc+1;
}

// (Indirect)
uint16_t SYSTEM65CORE System65::Addr_IND(void)
{
	// FIXME: Make sure this is correct
	// In memory, address is stored as: LSB MSB
	return (uint16_t)((uint16_t)memory[pc+2]+((uint16_t)memory[pc+1] << 8));
}

// (Indirect,X)
uint16_t SYSTEM65CORE System65::Addr_INX(void)
{
	// This is a doozy:
	// m[pc+1] is the immediate val to zp (1st mem access)
	// This is then added to x and masked to get a ptr to the zp with the LSB of
	// the actual pointer to return (2nd memory access).
	//uint16_t addr = memory[(memory[pc+1]+x)&0x00ff]
	//return (uint16_t)(((uint16_t)((addr)+1)<<4)+(uint16_t)(addr));
	uint16_t addr = memory[pc+1]+x;
	return (uint16_t)(((uint16_t)(memory[(addr+1)&0x00ff])<<8)+(uint16_t)(memory[addr&0x00ff]));
}

// (Indirect),Y
uint16_t SYSTEM65CORE System65::Addr_INY(void)
{
	// memory[pc+1] contains zp pointer
	// memory[memory[pc+1]] contains LSB of pointer
	// memory[memory[pc+1]]+1 contains MSB of pointer
	// pointer is added with y
	return (uint16_t)((uint16_t)((memory[memory[pc+1]+1])<<8)+(uint16_t)(memory[memory[pc+1]])+(uint16_t)y);
}

// Relative
uint16_t SYSTEM65CORE System65::Addr_REL(void){ assert(false); }

// Zeropage
uint16_t SYSTEM65CORE System65::Addr_ZPG(void)
{
	return (uint16_t)memory[pc+1];
}

// Zeropage,X
uint16_t SYSTEM65CORE System65::Addr_ZPX(void)
{
	return memory[(memory[pc+1]+x)&0x00ff];
}

// Zeropage,Y
uint16_t SYSTEM65CORE System65::Addr_ZPY(void)
{
	return memory[(memory[pc+1]+y)&0x00ff];
}
