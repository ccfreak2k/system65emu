#include "System65/System65.hpp"

// Address modes

// Each of these methods returns a 16-bit index (i.e. a pointer) into emulator
// memory for the operand of the instruction in question (this includes methods
// with uint8_t as their return type; the upper byte is assumed to be 0 in this
// case). For instance, Addr_IMM() returns a pointer to the next byte after the
// current opcode.

// The Accumulator (Addr_ACC) and Relative (Addr_REL) modes do not use a memory
// index, so calling the respective methods is an error. They are here simply
// for completion and as placeholders for possible future refactoring.

// Accumulator
uint8_t SYSTEM65CORE System65::Addr_ACC(void){ assert(false); return 0; }

// Absolute
uint16_t SYSTEM65CORE System65::Addr_ABS(void)
{
	return Memory_ReadWord(pc + 1);
}

// Absolute,X
uint16_t SYSTEM65CORE System65::Addr_ABX(void)
{
	return Memory_ReadWord(pc + 1) + x;
}

// Absolute,y
uint16_t SYSTEM65CORE System65::Addr_ABY(void)
{
	return Memory_ReadWord(pc + 1) + y;
}

// Immediate
uint16_t SYSTEM65CORE System65::Addr_IMM(void)
{
	return pc + 1;
}

// (Indirect)
uint16_t SYSTEM65CORE System65::Addr_IND(void)
{
	return Memory_ReadWord(Memory_ReadWord(pc + 1));
}

// (Indirect,X)
uint16_t SYSTEM65CORE System65::Addr_INX(void)
{
	return Memory_ReadWord((Memory_Read(pc + 1) + x) & 0xFF);
}

// (Indirect),Y
uint16_t SYSTEM65CORE System65::Addr_INY(void)
{
	return Memory_ReadWord(Memory_Read(pc + 1)) + y;
}

// Relative
uint16_t SYSTEM65CORE System65::Addr_REL(void){ assert(false); return 0; }

// Zeropage
uint8_t SYSTEM65CORE System65::Addr_ZPG(void)
{
	return Memory_Read(pc + 1);
}

// Zeropage,X
uint8_t SYSTEM65CORE System65::Addr_ZPX(void)
{
	return Memory_Read(pc + 1) + x;
}

// Zeropage,Y
uint8_t SYSTEM65CORE System65::Addr_ZPY(void)
{
	return Memory_Read(pc + 1) + y;
}
