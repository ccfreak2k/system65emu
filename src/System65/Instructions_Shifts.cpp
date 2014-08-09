#include "System65/System65.hpp"

// Shifts

#define LOCAL_ASL(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	addr = addrmode; \
	Helper_SetClear(System65::PFLAG_Z,(memory[addr] == 0)); \
	Helper_SetClear(System65::PFLAG_C,((memory[addr] & 0x80)!=0)); \
	memory[addr] *= 2; \
	Helper_SetClear(System65::PFLAG_N,((memory[addr] & 0x80)!=0)); \
	pc += isize

void SYSTEM65CORE System65::Insn_ASL(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t addr;
	switch(memory[pc]) {
	case 0x0a: // accumulator
		m_CycleCount += 2;
		Helper_SetClear(System65::PFLAG_Z,(a == 0));
		Helper_SetClear(System65::PFLAG_C,((a & 0x80)!=0));
		a *= 2;
		Helper_SetClear(System65::PFLAG_N,((a & 0x80)!=0));
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

#define LOCAL_LSR(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	addr = addrmode; \
	Helper_SetClear(System65::PFLAG_Z,(memory[addr] == 0)); \
	Helper_SetClear(System65::PFLAG_C,((memory[addr] & 0x80)!=0)); \
	memory[addr] /= 2; \
	Helper_SetClear(System65::PFLAG_N,((memory[addr] & 0x80)!=0)); \
	pc += isize

void SYSTEM65CORE System65::Insn_LSR(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t addr;
	switch (memory[pc]) {
	case 0x4a: // accumulator
		m_CycleCount += 2;
		Helper_SetClear(System65::PFLAG_Z,(a == 0));
		Helper_SetClear(System65::PFLAG_C,((a & 0x80)!=0));
		a /= 2;
		Helper_SetClear(System65::PFLAG_N,((a & 0x80)!=0));
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
			carry = ((a & 0x80) ? 1 : 0); \
			memory[addr] = (memory[addr] << 1 | (pf & System65::PFLAG_C ? 1 : 0)); \
			if (carry) \
				pf |= System65::PFLAG_C; \
			else \
				pf &= ~System65::PFLAG_C; \
			if (memory[addr] & 0x80) \
				pf |= System65::PFLAG_N; \
		} \
		pc += isize
void SYSTEM65CORE System65::Insn_ROL(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t addr;
	bool carry;
	switch (memory[pc]) {
	case 0x2a: // accumulator
		m_CycleCount += 2;
		if (a == 0)
			pf |= System65::PFLAG_Z;
		else {
			carry = ((a & 0x80) ? 1 : 0);
			a = (a << 1 | (pf & System65::PFLAG_C ? 1 : 0));

			if (carry)
				pf |= System65::PFLAG_C;
			else
				pf &= ~System65::PFLAG_C;

			if (a & 0x80)
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
			carry = ((memory[addr] & 0x01) ? 1 : 0); \
			memory[addr] = (memory[addr] >> 1 | (pf |= System65::PFLAG_C ? 1 : 0)); \
			if (carry) \
				pf |= System65::PFLAG_C; \
			else \
				pf &= ~System65::PFLAG_C; \
			if (memory[addr] & 0x80) \
				pf |= System65::PFLAG_N; \
		} \
		pc += isize
void SYSTEM65CORE System65::Insn_ROR(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
	uint16_t addr;
	bool carry;
	switch (memory[pc]) {
	case 0x6a: // accumulator
		m_CycleCount += 2;
		if (a == 0)
			pf |= System65::PFLAG_Z;
		else {
			carry = ((a & 0x01) ? 1 : 0);
			a = (a >> 1 | (pf |= System65::PFLAG_C ? 1 : 0));

			if (carry)
				pf |= System65::PFLAG_C;
			else
				pf &= ~System65::PFLAG_C;

			if (a & 0x80)
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
