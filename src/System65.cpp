#include "System65.hpp"

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

System65::System65(unsigned int memsize) :
	m_CycleCount(0),
	a(0x00),
	x(0x00),
	y(0x00),
	pf(System65::PFLAG_R),
	s(0x00),
	pc(0x0000)
{
	// Basic bounds checking
	if (memsize > 0x10000)
		memsize = 0x10000;

	if (memsize == 0)
		throw;

	memory = new uint8_t[0x10000];
#if _DEBUG
	assert(memory != NULL);
#endif // _DEBUG
	memset(memory,0,sizeof(uint8_t)*0xFFFF);
	memorysize = memsize;
}

System65::~System65()
{
	delete [] memory;
}

void System65::Tick(void)
{
	Dispatch();
}

void System65::Tick(unsigned int cycleLimit)
{
	// Start execution
	while (m_CycleCount < cycleLimit) {
		Dispatch();
	}

	// Reset the cycle count
	m_CycleCount -= cycleLimit;
}

uint8_t System65::GetRegister_A(void)
{
	ATOMIC_RETURN_8(a);
}

uint8_t System65::GetRegister_X(void)
{
	ATOMIC_RETURN_8(x);
}

uint8_t System65::GetRegister_Y(void)
{
	ATOMIC_RETURN_8(y);
}

uint8_t System65::GetRegister_P(void)
{
	ATOMIC_RETURN_8(pf);
}

uint8_t System65::GetRegister_S(void)
{
	ATOMIC_RETURN_8(s);
}

uint16_t System65::GetRegister_PC(void)
{
	ATOMIC_RETURN_16(pc);
}

//------------------------------------------------------------------------------
// Private
//------------------------------------------------------------------------------

void SYSTEM65CORE System65::Dispatch(void)
{
	m_mutMachine.lock();

	// Yes, a giant switch table. I know that it's a naive way to implement
	// this; however, it also serves as a good reference implementation, since
	// opcodes can be sorted arbitrarily; here I group them by function.
	switch (memory[pc]) {
	// ===========
	// LOAD/STORE
	// ===========
	case 0xa9: // LDA
	case 0xa5: //101xxx01
	case 0xb5:
	case 0xad:
	case 0xbd:
	case 0xb9:
	case 0xa1:
	case 0xb1:
		Insn_LDA();
		break;

	case 0xa2: // LDX
	case 0xa6: // 101xxx10
	case 0xb6:
	case 0xae:
	case 0xbe:
		Insn_LDX();
		break;

	case 0xa0: // LDY
	case 0xa4: // 101xxx00
	case 0xb4:
	case 0xac:
	case 0xbc:
		Insn_LDY();
		break;

	case 0x85: // STA
	case 0x95: // 100xxx01
	case 0x8d:
	case 0x9d:
	case 0x99:
	case 0x81:
	case 0x91:
		Insn_STA();
		break;

	case 0x86: // STX
	case 0x96: // 100xx110
	case 0x8e:
		Insn_STX();
		break;

	case 0x84: // STY
	case 0x94: // 100xx100
	case 0x8c:
		Insn_STY();
		break;

	// =================
	// REGISTER TRANSFER
	// =================
	case 0xaa: // TAX
		Insn_TAX();
		break;

	case 0xa8: // TAY
		Insn_TAY();
		break;

	case 0x8a: // TXA
		Insn_TXA();
		break;

	case 0x98: // TYA
		Insn_TYA();
		break;

	// ================
	// STACK OPERATIONS
	// ================
	case 0xba: // TSX
		Insn_TSX();
		break;

	case 0x9a: // TXS
		Insn_TXS();
		break;

	case 0x48: // PHA
		Insn_PHA();
		break;

	case 0x08: // PHP
		Insn_PHP();
		break;

	case 0x68: // PLA
		Insn_PLA();
		break;

	case 0x28: // PLP
		Insn_PLP();
		break;

	// ==================
	// LOGICAL OPERATIONS
	// ==================
	case 0x29: // AND
	case 0x25: // 0xxxxx01
	case 0x35:
	case 0x2d:
	case 0x3d:
	case 0x39:
	case 0x21:
	case 0x31:
		Insn_AND();
		break;

	case 0x49: // EOR
	case 0x45: // 010xxx01
	case 0x55:
	case 0x4d:
	case 0x5d:
	case 0x59:
	case 0x41:
	case 0x51:
		Insn_EOR();
		break;

	case 0x09: // ORA
	case 0x05: // 000xxx01
	case 0x15:
	case 0x0d:
	case 0x1d:
	case 0x19:
	case 0x01:
	case 0x11:
		Insn_ORA();
		break;

	case 0x24: // BIT
	case 0x2c: // 0010x10x
		Insn_BIT();
		break;

	// =====================
	// ARITHMETIC OPERATIONS
	// =====================

	case 0x69: // ADC
	case 0x65: // 011xxx01
	case 0x75:
	case 0x6d:
	case 0x7d:
	case 0x79:
	case 0x61:
	case 0x71:
		Insn_ADC();
		break;

	case 0xe9: // SBC
	case 0xe5: // 111xxx01
	case 0xf5:
	case 0xed:
	case 0xfd:
	case 0xf9:
	case 0xe1:
	case 0xf1:
		Insn_SBC();
		break;

	case 0xc9: // CMP
	case 0xc5: // 110xxx01
	case 0xd5:
	case 0xcd:
	case 0xdd:
	case 0xd9:
	case 0xc1:
	case 0xd1:
		Insn_CMP();
		break;

	case 0xe0: // CPX
	case 0xe4: // 1110xx00
	case 0xec:
		Insn_CPX();
		break;

	case 0xc0: // CPY
	case 0xc4: // 1100xx00
	case 0xcc:
		Insn_CPY();
		break;

	default:
		printf("Unhandled opcode 0x%.2X @ $%.4X\n",memory[pc],pc);
	}

#if _DEBUG
	assert(m_CycleCount != 0);
#endif // _DEBUG

	m_mutMachine.unlock();
}

// =============
// Address Modes
// =============

uint8_t SYSTEM65CORE System65::Addr_ACC(void){ assert(false); }

uint16_t SYSTEM65CORE System65::Addr_ABS(void)
{
	// FIXME: make sure types match and don't corrupt the retval
	// Further, the 6502 is little-endian so make sure the value is retrieved
	// correctly.
	return (uint16_t)((uint16_t)(memory[pc+2] << 4) + (uint16_t)memory[pc+1]);
}
uint16_t SYSTEM65CORE System65::Addr_ABX(void)
{
	return (uint16_t)((uint16_t)(memory[pc+2] << 4) + (uint16_t)memory[pc+1]) + x;
}
uint16_t SYSTEM65CORE System65::Addr_ABY(void)
{
	return (uint16_t)((uint16_t)(memory[pc+2] << 4) + (uint16_t)memory[pc+1]) + y;
}

uint16_t SYSTEM65CORE System65::Addr_IMM(void)
{
	return pc+1;
}

uint16_t SYSTEM65CORE System65::Addr_IND(void){ assert(false); }

uint16_t SYSTEM65CORE System65::Addr_INX(void)
{
	// This is a doozy:
	// m[pc+1] is the immediate val to zp (1st mem access)
	// This is then added to x and masked to get a ptr to the zp with the LSB of
	// the actual pointer to return (2nd memory access).
	//uint16_t addr = memory[(memory[pc+1]+x)&0x00ff]
	//return (uint16_t)(((uint16_t)((addr)+1)<<4)+(uint16_t)(addr));
	uint16_t addr = memory[pc+1]+x;
	return (uint16_t)(((uint16_t)(memory[(addr+1)&0x00ff])<<4)+(uint16_t)(memory[addr&0x00ff]));
}

uint16_t SYSTEM65CORE System65::Addr_INY(void)
{
	// memory[pc+1] contains zp pointer
	// memory[memory[pc+1]] contains LSB of pointer
	// memory[memory[pc+1]]+1 contains MSB of pointer
	// pointer is added with y
	return (uint16_t)((uint16_t)((memory[memory[pc+1]+1])<<4)+(uint16_t)(memory[memory[pc+1]])+(uint16_t)y);
}
uint16_t SYSTEM65CORE System65::Addr_REL(void){ assert(false); }

uint16_t SYSTEM65CORE System65::Addr_ZPG(void)
{
	return (uint16_t)memory[pc+1];
}

uint16_t SYSTEM65CORE System65::Addr_ZPX(void)
{
	return memory[(memory[pc+1]+x)&0x00ff];
}

uint16_t SYSTEM65CORE System65::Addr_ZPY(void)
{
	return memory[(memory[pc+1]+y)&0x00ff];
}

// ============
// Instructions
// ============

// Load/Store

void SYSTEM65CORE System65::Insn_LDA(void)
{
	switch (memory[pc]) {
	case 0xa9: // immediate
		INSN_M_TO_R(2,2,a,Addr_IMM()); break;
	case 0xa5: // zeropage
		INSN_M_TO_R(2,3,a,Addr_ZPG()); break;
	case 0xb5: // zeropage,x
		INSN_M_TO_R(2,4,a,Addr_ZPX()); break;
	case 0xad: // absolute
		INSN_M_TO_R(3,4,a,Addr_ABS()); break;
	case 0xbd: // absolute,x
		INSN_M_TO_R(3,4,a,Addr_ABX()); break;
	case 0xb9: // absolute,y
		INSN_M_TO_R(3,4,a,Addr_ABY()); break;
	case 0xa1: // (indirect,x)
		INSN_M_TO_R(2,6,a,Addr_INX()); break;
	case 0xb1: //(indirect),y
		INSN_M_TO_R(2,5,a,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	Insn_Set_ZN_Flags(a);
}

void SYSTEM65CORE System65::Insn_LDX(void)
{
	switch (memory[pc]) {
	case 0xa2: // immediate
		INSN_M_TO_R(2,2,x,Addr_IMM()); break;
	case 0xa6: // zeropage
		INSN_M_TO_R(2,3,x,Addr_ZPG()); break;
	case 0xb6: // zeropage,y
		INSN_M_TO_R(2,4,x,Addr_ZPY()); break;
	case 0xae: // absolute
		INSN_M_TO_R(3,4,x,Addr_ABS()); break;
	case 0xbe: // absolute,y
		INSN_M_TO_R(3,4,x,Addr_ABY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	Insn_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_LDY(void)
{
	switch (memory[pc]) {
	case 0xa0: // immediate
		INSN_M_TO_R(2,2,y,Addr_IMM()); break;
	case 0xa4: // zeropage
		INSN_M_TO_R(2,3,y,Addr_ZPG()); break;
	case 0xb4: // zeropage,x
		INSN_M_TO_R(2,4,y,Addr_ZPX()); break;
	case 0xac: // absolute
		INSN_M_TO_R(3,4,y,Addr_ABS()); break;
	case 0xbc: // absolute,x
		INSN_M_TO_R(3,4,y,Addr_ABX()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	Insn_Set_ZN_Flags(y);
}

void SYSTEM65CORE System65::Insn_STA(void)
{
	switch(memory[pc]) {
	case 0x85: // zeropage
		INSN_R_TO_M(2,3,a,Addr_ZPG()); break;
	case 0x95: // zeropage,x
		INSN_R_TO_M(2,4,a,Addr_ZPX()); break;
	case 0x8d: // absolute
		INSN_R_TO_M(3,4,a,Addr_ABS()); break;
	case 0x9d: // absolute,x
		INSN_R_TO_M(3,5,a,Addr_ABX()); break;
	case 0x99: // absolute,y
		INSN_R_TO_M(3,5,a,Addr_ABY()); break;
	case 0x81: // (indirect,x)
		INSN_R_TO_M(2,6,a,Addr_INX()); break;
	case 0x91: // (indirect),y
		INSN_R_TO_M(2,6,a,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}

void SYSTEM65CORE System65::Insn_STX(void)
{
	switch (memory[pc]) {
	case 0x86: // zeropage
		INSN_R_TO_M(2,3,x,Addr_ZPG()); break;
	case 0x96: // zeropage,y
		INSN_R_TO_M(2,4,x,Addr_ZPY()); break;
	case 0x8e: // absolute
		INSN_R_TO_M(3,4,x,Addr_ABS()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}

void SYSTEM65CORE System65::Insn_STY(void)
{
	switch (memory[pc]) {
	case 0x84: // zeropage
		INSN_R_TO_M(2,3,y,Addr_ZPG()); break;
	case 0x94: // zeropage,x
		INSN_R_TO_M(2,4,y,Addr_ZPX()); break;
	case 0x8c: // absolute
		INSN_R_TO_M(3,4,y,Addr_ABS()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}

// Register Transfer

void SYSTEM65CORE System65::Insn_TAX(void)
{
#if _DEBUG
	ASSERT_INSN(0xaa);
#endif // _DEBUG
	INSN_R_TO_R(1,2,x,a);
	Insn_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_TAY(void)
{
#if _DEBUG
	ASSERT_INSN(0xa8);
#endif // _DEBUG
	INSN_R_TO_R(1,2,y,a);
	Insn_Set_ZN_Flags(y);
}

void SYSTEM65CORE System65::Insn_TXA(void)
{
#if _DEBUG
	ASSERT_INSN(0x8a);
#endif // _DEBUG
	INSN_R_TO_R(1,2,a,x);
	Insn_Set_ZN_Flags(a);
}

void SYSTEM65CORE System65::Insn_TYA(void)
{
#if _DEBUG
	ASSERT_INSN(0x98);
#endif // _DEBUG
	INSN_R_TO_R(1,2,a,y);
	Insn_Set_ZN_Flags(a);
}

// Stack Operations

void SYSTEM65CORE System65::Insn_TSX(void)
{
#if _DEBUG
	ASSERT_INSN(0xba);
#endif // _DEBUG
	INSN_R_TO_R(1,2,x,s);
	Insn_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_TXS(void)
{
#if _DEBUG
	ASSERT_INSN(0x9a);
#endif // _DEBUG
	INSN_R_TO_R(1,2,s,x);
}

void SYSTEM65CORE System65::Insn_PHA(void)
{
#if _DEBUG
	ASSERT_INSN(0x48);
#endif // _DEBUG
	m_CycleCount += 3;
	memory[--s] = a;
	pc += 1;
}

void SYSTEM65CORE System65::Insn_PHP(void)
{
#if _DEBUG
	ASSERT_INSN(0x08);
#endif // _DEBUG
	m_CycleCount += 3;
	memory[--s] = pf;
	pc += 1;
}

void SYSTEM65CORE System65::Insn_PLA(void)
{
#if _DEBUG
	ASSERT_INSN(0x68);
#endif // _DEBUG
	m_CycleCount += 4;
	a = memory[s++];
	Insn_Set_ZN_Flags(a);
	pc += 1;
}

void SYSTEM65CORE System65::Insn_PLP(void)
{
#if _DEBUG
	ASSERT_INSN(0x28);
#endif // _DEBUG
	m_CycleCount += 4;
	pf = memory[s++]|System65::PFLAG_R;
	pc += 1;
}

// Logical Operations
#define LOCAL_AND(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	a &= memory[addrmode]; \
	Insn_Set_ZN_Flags(a); \
	pc += isize
void SYSTEM65CORE System65::Insn_AND(void)
{
	switch(memory[pc]) {
	case 0x29: // immediate
		LOCAL_AND(2,2,Addr_IMM()); break;
	case 0x25: // zeropage
		LOCAL_AND(2,3,Addr_ZPG()); break;
	case 0x35: // zeropage,x
		LOCAL_AND(2,4,Addr_ZPX()); break;
	case 0x2d: // absolute
		LOCAL_AND(3,4,Addr_ABS()); break;
	case 0x3d: // absolute,x
		LOCAL_AND(3,4,Addr_ABX()); break;
	case 0x39: // absolute,y
		LOCAL_AND(3,4,Addr_ABY()); break;
	case 0x21: // (indirect,x)
		LOCAL_AND(2,6,Addr_INX()); break;
	case 0x31: // (indirect),y
		LOCAL_AND(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}
#undef LOCAL_AND

#define LOCAL_EOR(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	a ^= memory[addrmode]; \
	Insn_Set_ZN_Flags(a); \
	pc += isize
void SYSTEM65CORE System65::Insn_EOR(void)
{
	switch(memory[pc]) {
	case 0x49: // immediate
		LOCAL_EOR(2,2,Addr_IMM()); break;
	case 0x45: // zeropage
		LOCAL_EOR(2,3,Addr_ZPG()); break;
	case 0x55: // zeropage,x
		LOCAL_EOR(2,4,Addr_ZPX()); break;
	case 0x4d: // absolute
		LOCAL_EOR(3,4,Addr_ABS()); break;
	case 0x5d: // absolute,x
		LOCAL_EOR(3,4,Addr_ABX()); break;
	case 0x59: // absolute,y
		LOCAL_EOR(3,4,Addr_ABY()); break;
	case 0x41: // (indirect,x)
		LOCAL_EOR(2,6,Addr_INX()); break;
	case 0x51: // (indirect),y
		LOCAL_EOR(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}
#undef LOCAL_EOR

#define LOCAL_ORA(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	a |= memory[addrmode]; \
	Insn_Set_ZN_Flags(a); \
	pc += isize
void SYSTEM65CORE System65::Insn_ORA(void)
{
	switch(memory[pc]) {
	case 0x09: // immediate
		LOCAL_ORA(2,2,Addr_IMM()); break;
	case 0x05: // zeropage
		LOCAL_ORA(2,3,Addr_IMM()); break;
	case 0x15: // zeropage,x
		LOCAL_ORA(2,4,Addr_IMM()); break;
	case 0x0d: // absolute
		LOCAL_ORA(3,4,Addr_IMM()); break;
	case 0x1d: // absolute,x
		LOCAL_ORA(3,4,Addr_IMM()); break;
	case 0x19: // absolute,y
		LOCAL_ORA(3,4,Addr_IMM()); break;
	case 0x01: // (indirect,x)
		LOCAL_ORA(2,6,Addr_IMM()); break;
	case 0x11: // (indirect),y
		LOCAL_ORA(2,5,Addr_IMM()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}
}
#undef LOCAL_ORA

void SYSTEM65CORE System65::Insn_BIT(void)
{
	uint8_t val;
	switch (memory[pc]) {
	case 0x24: // zeropage
		m_CycleCount += 3;
		val = memory[Addr_ZPG()];
	case 0x2c: // absolute
		m_CycleCount += 4;
		val = memory[Addr_ABS()];
	default:
		INSN_DECODE_ERROR(); return;
	}
	pc += 2;

	if (a & val)
		pf &= ~System65::PFLAG_Z;
	else
		pf |= System65::PFLAG_Z;

    // TODO: Faster/more concise bit setting
    if (val & 0b10000000)
		pf |= System65::PFLAG_N;
	else
		pf &= ~System65::PFLAG_N;

	if (val & 0b01000000)
		pf |= System65::PFLAG_V;
	else
		pf &= ~System65::PFLAG_V;
}

// Arithemtic Operations

#define LOCAL_LOADVAL(isize,ccount,addrmode) \
	m_CycleCount += ccount; \
	val = memory[addrmode]; \
	pc += isize
void SYSTEM65CORE System65::Insn_ADC(void)
{
	// TODO: BCD mode add
	uint16_t val;
	switch(memory[pc]) {
	case 0x69: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0x65: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0x75: // zeropage,x
		LOCAL_LOADVAL(2,4,Addr_ZPX()); break;
	case 0x6d: // absolute
		LOCAL_LOADVAL(3,4,Addr_ABS()); break;
	case 0x7d: // absolute,x
		LOCAL_LOADVAL(3,4,Addr_ABX()); break;
	case 0x79: // absolute,y
		LOCAL_LOADVAL(3,4,Addr_ABY()); break;
	case 0x61: // (indirect,x)
		LOCAL_LOADVAL(2,6,Addr_INX()); break;
	case 0x71: // (indirect),y
		LOCAL_LOADVAL(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	// Add w/ carry
	uint16_t nval = a + val + ((pf & System65::PFLAG_C) ? 1 : 0);

	// Set flags
	if (nval > 0xff) // carry
		pf |= System65::PFLAG_C;

	if (nval == 0) // zero
		pf |= System65::PFLAG_Z;

	if (((nval^a)&(nval^val)&0x80) > 0) // overflow
		pf |= System65::PFLAG_V;

	if (nval & 0b10000000) // negative
		pf |= System65::PFLAG_N;

	// write
	a = (uint8_t)nval;
}

void SYSTEM65CORE System65::Insn_SBC(void)
{
	// TODO: BCD mode subtract
	uint16_t val;
	switch(memory[pc]) {
	case 0xe9: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xe5: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xf5: // zeropage,x
		LOCAL_LOADVAL(2,4,Addr_ZPX()); break;
	case 0xed: // absolute
		LOCAL_LOADVAL(3,4,Addr_ABS()); break;
	case 0xfd: // absolute,x
		LOCAL_LOADVAL(3,4,Addr_ABX()); break;
	case 0xf9: // absolute,y
		LOCAL_LOADVAL(3,4,Addr_ABY()); break;
	case 0xe1: // (indirect,x)
		LOCAL_LOADVAL(2,6,Addr_INX()); break;
	case 0xd1: // (indirect),y
		LOCAL_LOADVAL(2,5,Addr_INY()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	// subtract w/ carry
	uint16_t nval = a - val + ((pf & System65::PFLAG_C) ? 1 : 0) - 1;

	// Set flags
	if (!(nval&0x100)) // carry
		pf |= System65::PFLAG_C;

	if (nval == 0) // zero
		pf |= System65::PFLAG_Z;

	if (((nval^a)&(nval^-val)&0x80) > 0) // Overflow
		pf |= System65::PFLAG_V;

	if (nval & 0b10000000) // negative
		pf |= System65::PFLAG_N;

	// write
	a = (uint8_t)nval;
}

void SYSTEM65CORE System65::Insn_CMP(void)
{
	uint8_t val;
	switch (memory[pc]) {
	case 0xc9: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xc5: // zeropage
		LOCAL_LOADVAL(2,3,Addr_IMM()); break;
	case 0xd5: // zeropage,x
		LOCAL_LOADVAL(2,4,Addr_IMM()); break;
	case 0xcd: // absolute
		LOCAL_LOADVAL(3,4,Addr_IMM()); break;
	case 0xdd: // absolute,x
		LOCAL_LOADVAL(3,4,Addr_IMM()); break;
	case 0xd9: // absolute,y
		LOCAL_LOADVAL(3,4,Addr_IMM()); break;
	case 0xc1: // (indirect,x)
		LOCAL_LOADVAL(2,6,Addr_IMM()); break;
	case 0xd1: // (indirect),y
		LOCAL_LOADVAL(2,5,Addr_IMM()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	if (a >= val) // carry
		pf |= System65::PFLAG_C;

	if (a == val) // zero
		pf |= System65::PFLAG_Z;

	if ((a - val) < 0) // negative
		pf |= System65::PFLAG_N;
}

void SYSTEM65CORE System65::Insn_CPX(void)
{
	uint8_t val;
	switch(memory[pc]) {
	case 0xe0: // immediate
		LOCAL_LOADVAL(2,2,Addr_ABS()); break;
	case 0xe4: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xec: // absolute
		LOCAL_LOADVAL(3,4,Addr_ABS()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	if (x >= val) // carry
		pf |= System65::PFLAG_C;

	if (x == val) // zero
		pf |= System65::PFLAG_Z;

	if ((x - val) < 0) // negative
		pf |= System65::PFLAG_N;
}

void SYSTEM65CORE System65::Insn_CPY(void)
{
	uint8_t val;
	switch(memory[pc]) {
	case 0xc0: // immediate
		LOCAL_LOADVAL(2,2,Addr_IMM()); break;
	case 0xc4: // zeropage
		LOCAL_LOADVAL(2,3,Addr_ZPG()); break;
	case 0xcc: // absolute
		LOCAL_LOADVAL(2,3,Addr_ABS()); break;
	default:
		INSN_DECODE_ERROR(); return;
	}

	if (y >= val) // carry
		pf |= System65::PFLAG_C;

	if (y == val) // zero
		pf |= System65::PFLAG_Z;

	if ((y - val) < 0) // negative
		pf |= System65::PFLAG_N;
}
#undef LOCAL_LOADVAL

// =================
// EXECUTION HELPERS
// =================

void SYSTEM65CORE System65::Insn_Set_ZN_Flags(uint8_t reg)
{
	if (reg == 0)
		pf |= System65::PFLAG_Z;
	else if (reg & 0b10000000)
		pf |= System65::PFLAG_N;
}
