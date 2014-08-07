#include "System65/System65.hpp"

// Address modes

// Each of these methods returns a 16-bit index (i.e. a pointer) into emulator
// memory for the operand of the instruction in question.

// The Accumulator (Addr_ACC) and Relative (Addr_REL) modes do not use a memory
// index, so calling the respective methods is an error. They are here simply
// for completion and as placeholders for possible future refactoring.

// Accumulator
uint8_t SYSTEM65CORE System65::Addr_ACC(void){ assert(false); return 0; }

// Absolute
uint16_t SYSTEM65CORE System65::Addr_ABS(void)
{
	// FIXME: JMP doesn't use this as a pointer (see insn 0x4c); make sure
	// other callers are using it correctly as well.
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
	return (uint16_t)((uint16_t)memory[pc+1]+((uint16_t)memory[pc+2] << 8));
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
	return (uint16_t)(((uint16_t)(memory[(addr+1)&0xff])<<8)+(uint16_t)(memory[addr&0xff]));
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
uint16_t SYSTEM65CORE System65::Addr_REL(void){ assert(false); return 0; }

// Zeropage
uint16_t SYSTEM65CORE System65::Addr_ZPG(void)
{
	return (uint16_t)memory[pc+1];
}

// Zeropage,X
uint16_t SYSTEM65CORE System65::Addr_ZPX(void)
{
	return memory[(memory[pc+1]+x)&0xff];
}

// Zeropage,Y
uint16_t SYSTEM65CORE System65::Addr_ZPY(void)
{
	return memory[(memory[pc+1]+y)&0xff];
}
