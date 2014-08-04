#include "System65/System65.hpp"

// Load/Store

void SYSTEM65CORE System65::Insn_LDA(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
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

	Helper_Set_ZN_Flags(a);
}

void SYSTEM65CORE System65::Insn_LDX(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
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

	Helper_Set_ZN_Flags(x);
}

void SYSTEM65CORE System65::Insn_LDY(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
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

	Helper_Set_ZN_Flags(y);
}

void SYSTEM65CORE System65::Insn_STA(void)
{
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
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
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
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
#ifdef DEBUG_PRINT_INSTRUCTION
	PRINT_INSTRUCTION();
#endif // DEBUG_PRINT_INSTRUCTION
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
