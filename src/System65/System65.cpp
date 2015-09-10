#include "System65/System65.hpp"

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

System65::System65(unsigned int memsize) :
	m_CycleCount(0),
	m_StackBase(STACK_BASE),
	m_GenerateInterrupt(false),
	m_NMInterrupt(false),
	m_InterruptVector(0x0000),
	m_BreakFlagSet(false),
	a(0x00),
	x(0x00),
	y(0x00),
	pf(System65::PFLAG_R|System65::PFLAG_I),
	s(0xFD),
	pc(CODE_BASE),
	memorysize(memsize)
{
	// Basic bounds checking
	if (memsize > MAX_MEM_SIZE)
		memsize = MAX_MEM_SIZE;

	if (memsize == 0)
		throw;

	m_Memory = std::make_unique<std::vector<uint8_t>>(MAX_MEM_SIZE);

	//m_Trace = std::make_unique<Trace::BinaryRecord>();

	m_TraceMemory = std::make_shared<std::vector<uint8_t>>(MAX_MEM_SIZE);
}

System65::~System65()
{
}

// TODO: fix this or just remove this method completely
void System65::LoadProgram(void *progmem, unsigned int progsize, unsigned int offset)
{
	if (progmem == NULL)
		return;
	if (progsize == 0)
		return;

	uint8_t *m = static_cast<uint8_t*>(progmem);

	//memcpy(&m_Memory[offset],(uint8_t*)progmem,(size_t)((65536-offset) < progsize ? (65536-offset) : progsize));
	for (int i = offset; (i < MAX_MEM_SIZE) && ((i - offset) <= progsize); i++) {
		Memory_Write(i, *m);
	}

	Reset();
	pc = CODE_BASE;
}

// TODO: More descriptive throws
void System65::LoadProgram(std::string filename, unsigned int offset)
{
	unsigned int size = MAX_MEM_SIZE - offset;

	std::ifstream file(filename, std::ios::binary);
	if (!file.good())
		throw;

	std::vector<uint8_t> prog(size);
	file.read(reinterpret_cast<char*>(prog.data()), size);

	if (!file.good() && !file.eof())
		throw;

	for (uint16_t i = 0; (i < MAX_MEM_SIZE - CODE_BASE) && (i < size); i++)
		Memory_Write(i+offset, prog[i]);

	Reset();
	pc = CODE_BASE;
}

void System65::Reset(void)
{
	Helper_SetFlag(System65::PFLAG_I);
	Helper_ClearFlag(System65::PFLAG_D);
	pc = Memory_ReadWord(0xFFFC);
	s = 0xFD;
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

void System65::Interrupt(bool nmi)
{
	Helper_SetInterrupt(nmi, false);
}

void System65::SetStackBasePage(uint8_t base)
{
	m_StackBase = (uint16_t)base << 8;
}

void System65::SetInterruptVector(uint16_t ivec)
{
	Memory_Write(0xFFFE,ivec);
}

//------------------------------------------------------------------------------
// Private
//------------------------------------------------------------------------------

void SYSTEM65CORE System65::Dispatch(void)
{
#if _DEBUG
	unsigned int oldcyclecount = m_CycleCount;
	uint16_t oldpc = pc;

	static bool start_clock = false;
	static unsigned int curcyclecount; // Cycles since last reset

	// Start the clock
	if (!start_clock) {
		start_clock = true;
		curcyclecount = 0;
		m_CStart = std::clock();
	}
#endif // _DEBUG

	// Service any pending interrupts first
	// TODO: Check to make sure Helper_HandleInterrupt() is called appropriately.
	if (!(m_GenerateInterrupt && Helper_HandleInterrupt())) {
		// Yes, a giant switch table. I know that it's a naive way to implement
		// this; however, it also serves as a good reference implementation, since
		// opcodes can be sorted arbitrarily; here I group them by function.
		switch (Memory_Read(pc)) {
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

		// ===================
		// INCREMENT/DECREMENT
		// ===================

		case 0xe6: // INC
		case 0xf6: //
		case 0xee:
		case 0xfe:
			Insn_INC();
			break;

		case 0xe8: // INX
			Insn_INX();
			break;

		case 0xc8: // INY
			Insn_INY();
			break;

		case 0xc6: // DEC
		case 0xd6: //
		case 0xce:
		case 0xde:
			Insn_DEC();
			break;

		case 0xca: // DEX
			Insn_DEX();
			break;

		case 0x88: // DEY
			Insn_DEY();
			break;

		// ======
		// SHIFTS
		// ======

		case 0x0a: // ASL
		case 0x06: //
		case 0x16:
		case 0x0e:
		case 0x1e:
			Insn_ASL();
			break;

		case 0x4a: // LSR
		case 0x46: //
		case 0x56:
		case 0x4e:
		case 0x5e:
			Insn_LSR();
			break;

		case 0x2a: // ROL
		case 0x26: //
		case 0x36:
		case 0x2e:
		case 0x3e:
			Insn_ROL();
			break;

		case 0x6a: // ROR
		case 0x66: //
		case 0x76:
		case 0x6e:
		case 0x7e:
			Insn_ROR();
			break;

		// ===========
		// JUMPS/CALLS
		// ===========

		case 0x4c: // JMP
		case 0x6c: //
			Insn_JMP();
			break;

		case 0x20: // JSR
			Insn_JSR();
			break;

		case 0x60: // RTS
			Insn_RTS();
			break;

		// ========
		// BRANCHES
		// ========

		case 0x90: // BCC
			Insn_BCC();
			break;

		case 0xb0: // BCS
			Insn_BCS();
			break;

		case 0xf0: // BEQ
			Insn_BEQ();
			break;

		case 0x30: // BMI
			Insn_BMI();
			break;

		case 0xd0: // BNE
			Insn_BNE();
			break;

		case 0x10: // BPL
			Insn_BPL();
			break;

		case 0x50: // BVC
			Insn_BVC();
			break;

		case 0x70: // BVS
			Insn_BVS();
			break;

		// ======================
		// STATUS FLAG OPERATIONS
		// ======================

		case 0x18: // CLC
			Insn_CLC();
			break;

		case 0xd8: // CLD
			Insn_CLD();
			break;

		case 0x58: // CLI
			Insn_CLI();
			break;

		case 0xb8: // CLV
			Insn_CLV();
			break;

		case 0x38: // SEC
			Insn_SEC();
			break;

		case 0xf8: // SED
			Insn_SED();
			break;

		case 0x78: // SEI
			Insn_SEI();
			break;

		// =================
		// SYSTEM OPERATIONS
		// =================

		case 0x00: // BRK
			Insn_BRK();
			break;

		case 0xea: // NOP
			Insn_NOP();
			break;

		case 0x40: // RTI
			Insn_RTI();
			break;

		default:
			printf("Unhandled opcode 0x%.2X @ $%.4X\n",Memory_Read(pc),pc);
			m_InstructionCount--;
		}

		m_InstructionCount++;

		// Copy memory for the snapshot
		//for (unsigned i = 0; i < MAX_MEM_SIZE; i++)
		//	(*m_TraceMemory)[i] = Memory_Read(i);

		//m_Trace->Snap(m_InstructionCount, a, x, y, pf, s, pc, m_TraceMemory);

		// 0x3264 is the value of pc if the functional test passes
		//if (pc == 0x3264)
		//	exit(0);
	}

#if _DEBUG
	if (!m_GenerateInterrupt) {
		assert(oldcyclecount != m_CycleCount);
		assert(oldpc != pc);
	}
	curcyclecount += m_CycleCount - oldcyclecount;
	if (curcyclecount >= 1000000) {
		m_CStop = std::clock();
		float time = 1000.0f * (m_CStop - m_CStart) / CLOCKS_PER_SEC;
		std::cout << "1M cycles in " << time << "ms (" << 1/time*1000 << "MHz)" << std::endl;
		curcyclecount -= 1000000;
		start_clock = false;
	}
#endif // _DEBUG
}
