#include "System65/System65.hpp"

// Shifts

#define LOCAL_ASL(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	addr = addrmode; \
	if (memory[addr] == 0) \
		pf |= System65::PFLAG_Z; \
	else {\
			if (memory[addr] & 0b10000000) \
				pf |= System65::PFLAG_C; \
			memory[addr] *= 2; \
			if (memory[addr] & 0b10000000) \
				pf |= System65::PFLAG_N; \
	} \
	pc += isize
void SYSTEM65CORE System65::Insn_ASL(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t addr;
	switch(memory[pc]) {
	case 0x0a: // accumulator
		m_CycleCount += 2;
		if (a == 0)
			pf |= System65::PFLAG_Z;
		else {
			if (a & 0b10000000)
				pf |= System65::PFLAG_C;
			a *= 2;
			if (a & 0b10000000)
				pf |= System65::PFLAG_N;
		}
		pc += 1;
		break;
	case 0x06: // zeropage
		LOCAL_ASL(2,5,Addr_ZPG()); break;
	case 0x16: // zeropage,x
		LOCAL_ASL(2,6,Addr_ZPX()); break;
	case 0x0e: // absolute
		LOCAL_ASL(3,6,Addr_ABS()); break;
	case 0x1e: // absolute,x
		LOCAL_ASL(3,7,Addr_ABX()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}
#undef LOCAL_ASL

// TODO: Does C need to be cleared too?
#define LOCAL_LSR(isize,ccount,addrmode) \
		m_CycleCount += ccount; \
		addr = addrmode; \
		if (memory[addr] == 0) \
			pf |= System65::PFLAG_Z; \
		else { \
			if (memory[addr] & 0b00000001) \
				pf |= System65::PFLAG_C; \
			memory[addr] /= 2;  \
			if (memory[addr] & 0b10000000) \
				pf |= System65::PFLAG_N; \
		} \
		pc += isize
void SYSTEM65CORE System65::Insn_LSR(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t addr;
	switch (memory[pc]) {
	case 0x4a: // accumulator
		m_CycleCount += 2;
		if (a == 0)
			pf |= System65::PFLAG_Z;
		else {
			if (a & 0b00000001)
				pf |= System65::PFLAG_C;
			a /= 2;
			if (a & 0b10000000)
				pf |= System65::PFLAG_N;
		}
		pc += 1;
		break;
	case 0x46: // zeropage
		LOCAL_LSR(2,5,Addr_ZPG()); break;
	case 0x56: // zeropage,x
		LOCAL_LSR(2,6,Addr_ZPX()); break;
	case 0x4e: // absolute
		LOCAL_LSR(3,6,Addr_ABS()); break;
	case 0x5e: // absolute,x
		LOCAL_LSR(3,7,Addr_ABX()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}
#undef LOCAL_LSR

#define LOCAL_ROL(isize,ccount,addrmode) \
		m_CycleCount += ccount; \
		addr = addrmode; \
		if (memory[addr] == 0) \
			pf |= System65::PFLAG_Z; \
		else { \
			carry = ((a & 0b10000000) ? 1 : 0); \
			memory[addr] = (memory[addr] << 1 | (pf & System65::PFLAG_C ? 1 : 0)); \
			if (carry) \
				pf |= System65::PFLAG_C; \
			else \
				pf &= ~System65::PFLAG_C; \
			if (memory[addr] & 0b10000000) \
				pf |= System65::PFLAG_N; \
		} \
		pc += isize
void SYSTEM65CORE System65::Insn_ROL(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t addr;
	bool carry;
	switch (memory[pc]) {
	case 0x2a: // accumulator
		m_CycleCount += 2;
		if (a == 0)
			pf |= System65::PFLAG_Z;
		else {
			carry = ((a & 0b10000000) ? 1 : 0);
			a = (a << 1 | (pf & System65::PFLAG_C ? 1 : 0));

			if (carry)
				pf |= System65::PFLAG_C;
			else
				pf &= ~System65::PFLAG_C;

			if (a & 0b10000000)
				pf |= System65::PFLAG_N;
		}
		pc += 1;
		break;
	case 0x26: // zeropage
		LOCAL_ROL(2,5,Addr_ZPG()); break;
	case 0x36: // zeropage,x
		LOCAL_ROL(2,6,Addr_ZPX()); break;
	case 0x2e: // absolute
		LOCAL_ROL(3,6,Addr_ABS()); break;
	case 0x3e: // absolute,x
		LOCAL_ROL(3,7,Addr_ABX()); break;
	default:
		INSN_DECODE_ERROR(); break;
	}
}
#undef LOCAL_ROL

#define LOCAL_ROR(isize,ccount,addrmode) \
		m_CycleCount += ccount; \
		addr = addrmode; \
		if (memory[addr] == 0) \
			pf |= System65::PFLAG_Z; \
		else { \
			carry = ((memory[addr] & 0b00000001) ? 1 : 0); \
			memory[addr] = (memory[addr] >> 1 | (pf |= System65::PFLAG_C ? 1 : 0)); \
			if (carry) \
				pf |= System65::PFLAG_C; \
			else \
				pf &= ~System65::PFLAG_C; \
			if (memory[addr] & 0b10000000) \
				pf |= System65::PFLAG_N; \
		} \
		pc += isize
void SYSTEM65CORE System65::Insn_ROR(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc);
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t addr;
	bool carry;
	switch (memory[pc]) {
	case 0x6a: // accumulator
		m_CycleCount += 2;
		if (a == 0)
			pf |= System65::PFLAG_Z;
		else {
			carry = ((a & 0b00000001) ? 1 : 0);
			a = (a >> 1 | (pf |= System65::PFLAG_C ? 1 : 0));

			if (carry)
				pf |= System65::PFLAG_C;
			else
				pf &= ~System65::PFLAG_C;

			if (a & 0b10000000)
				pf |= System65::PFLAG_N;
		}
		pc += 1;
		break;
	case 0x66: // zeropage
		LOCAL_ROR(2,5,Addr_ZPG()); break;
	case 0x76: // zeropage,x
		LOCAL_ROR(2,6,Addr_ZPX()); break;
	case 0x6e: // absolute
		LOCAL_ROR(3,6,Addr_ABS()); break;
	case 0x7e: // absoute,x
		LOCAL_ROR(3,7,Addr_ABX()); break;
	default:
		INSN_DECODE_ERROR(); break;
	}
}
