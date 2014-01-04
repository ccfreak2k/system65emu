#ifndef SYSTEM65_HPP
#define SYSTEM65_HPP

// Standard libs
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Class-related libs
#include <SFML/System/Mutex.hpp>

/** \file System65.hpp
 * Interface for the \ref System65 class.
 */

/**
 * The System65 class.
 *
 * This class is the class that represents a System65 computer as a whole. It
 * encompasses everything that a System65 computer needs to run, including its
 * memory, CPU, execution state, etc.
 *
 * This class contains a full 6502 core with customizations specific to
 * System65.
 *
 * System65 system memory is internally allocated as a single 64KB block of
 * data; specifying a smaller amount when instancing merely changes
 * \ref memorysize. Accesses to memory "outside" of this range is allowed to
 * allow DMA to devices such as the screen.
 *
 * The machine can be set to execute only a certain number of cycles at a time.
 * By calling \ref Tick with a value, the machine will run until that number of
 * cycles has been run, at which point it will return. The machine state will
 * be preserved, and simply calling \ref Tick again will allow it to continue.
 * By periodically sampling the machine state, it is possible to follow
 * execution to a certain extent. Passing a value of 0 will cause \ref Tick to
 * return immediately, and no instructions will be executed.
 *
 * \author ccfreak2k
 *
 * \todo Add mutex to lock machine state while running (e.g. GetRegister_*()
 * calls block)
 *
 * \todo Increment the cycle count in each stage of execution (allows for
 * accurate cycle penalty for cross-page reads).
 */

/** \def FASTEMU When defined, functions will be inlined for speed. May increase code size heavily. */
#ifdef FASTEMU
	#define SYSTEM65CORE inline
#else
	#define SYSTEM65CORE
#endif

/** Function macro to assert the correct decoding of an instruction */
#define ASSERT_INSN(byte) assert(memory[pc] == byte)

/** Function macro for writing a value from memory to a register */
#define INSN_R_TO_M(isize,ccount,reg,addrmode) \
	m_CycleCount += ccount; \
	reg = memory[addrmode]; \
	pc += isize

/** Function macro for writing a value from a register to memory */
#define INSN_M_TO_R(isize,ccount,reg,addrmode) \
	m_CycleCount += ccount; \
	memory[addrmode] = reg; \
	pc += isize

/** Function macro for copying a value from one register to another */
#define INSN_R_TO_R(isize,ccount,regdst,regsrc) \
	m_CycleCount += ccount; \
	regdst = regsrc; \
	pc += isize

/** Function macro for reporting an erroneous instruction decode */
#define INSN_DECODE_ERROR() \
	printf("ERROR: %s called with opcode 0x%.2X\n", __FUNCTION__, memory[pc])

/** Function macro for returning an 8-bit value */
#define ATOMIC_RETURN_8(val) \
	m_mutMachine.lock(); \
	uint8_t retval = val; \
	m_mutMachine.unlock(); \
	return retval

/** Function macro for returning a 16-bit value */
#define ATOMIC_RETURN_16(val) \
	m_mutMachine.lock(); \
	uint16_t retval = val; \
	m_mutMachine.unlock(); \
	return retval

class System65
{
	public:
		/** Creates a System65 context
		 *
		 * \param[in] memsize Amount of memory, in bytes, for this machine, up
		 * to 64KB. Values greater than 64KB are clamped; a value of 0 will
		 * throw.
		 *
		 * \todo Throw a specific exception
		 */
		System65(unsigned int memsize = 0x1000);

		/** Destroys the System65 context */
		~System65();

		/** Runs 1 instruction of the CPU and returns.
		 *
		 * \note This is currently not a cycle-accurate emulation; an executed
		 * instruction may take multiple cycles to run
		 */
		void Tick(void);

		/** Runs a number of instructions of the CPU and returns.
		 *
		 * \param[in] cycleLimit Number of consecutive cycles to run before stopping
		 *
		 * \note A count of less than 1 returns immediately.
		 * \note The executed cycle count before returning will be <em>at
		 * least</em> \c cycleLimit. If if any extra cycles are taken to finish
		 * executing the last instruction, they wll be added to the next cycle
		 * count so that, on average, the executed count will be close to the
		 * specified limit. There is currently no mechanism to prevent any
		 * single instruction execution from exceeding \c cycleLimit.
		 */
		void Tick(unsigned int cycleLimit);

		/** Returns the contents of the accumulator register. */
		uint8_t GetRegister_A(void);

		/** Returns the contents of the X index register. */
		uint8_t GetRegister_X(void);

		/** Returns the contents of the Y index register. */
		uint8_t GetRegister_Y(void);

		/** Returns the contents of the processor flags register. */
		uint8_t GetRegister_P(void);

		/** Returns the contents of the stack pointer register. */
		uint8_t GetRegister_S(void);

		/** Returns the contents of the program counter register. */
		uint16_t GetRegister_PC(void);

		/** Resets the cycle counter. */
		void ResetCycleCount(void) { m_CycleCount = 0; }

		uint8_t *memory; //!< Pointer to system memory for this system
		unsigned int memorysize; //!< Size of memory for this system.

		sf::Mutex m_mutMachine; //!< Mutex protecting the machine state.

	protected:
	private:
		unsigned int m_CycleCount; //!< Tracks the number of cycles executed so far.

		/** \defgroup 65regs 6502 Registers
		 *
		 * This is the set of registers available in the System65 CPU core.
		 * @{
		 */
		uint8_t a; //!< Accumulator
		uint8_t x; //!< X index
		uint8_t y; //!< Y index
		uint8_t pf; //!< Processor status flags
		uint8_t s; //!< Stack pointer
		uint16_t pc; //!< Program counter
		/** @} */

		/** Processor flags */
		enum PFLAGS {
			PFLAG_C = 0x01, //!< Carry flag
			PFLAG_Z = 0x02, //!< Result-is-zero flag
			PFLAG_I = 0x04, //!< Interrupt (IRQ disable)
			PFLAG_D = 0x08, //!< Decimal mode flag
			PFLAG_B = 0x10, //!< Break
			PFLAG_R = 0x20, //!< Reserved (always 1)
			PFLAG_V = 0x40, //!< Overflow flag
			PFLAG_N = 0x80  //!< Negative flag
		};

		/** \defgroup addressmodes  Memory addressing modes
		 *
		 * These functions translate input address data and mode and return the
		 * actual address to read/write. Each specific opcode that reads from or
		 * writes to an address invokes one of these, but these functions read
		 * directly from the machine state.
		 *
		 * Unless otherwise noted, each function returns an \em index to the memory
		 * array as a \c uint8_t, which can be treated as an absolute pointer in
		 * memory.
		 *
		 * \note Each function returns 16-bit addresses in big-endian (MSB
		 * first) to index the memory array correctly.
		 *
		 * \note Most function descriptions in this group taken from this web
		 * page: http://www.obelisk.demon.co.uk/6502/addressing.html
		 * @{
		 */

		/**
		 * Some instructions have an option to operate directly upon the
		 * accumulator. The programmer specifies this by using a special operand
		 * value, 'A'. For example:
		 *
		 * \code{.asm}
		LSR A           ;Logical shift right one bit
        ROR A           ;Rotate right one bit
		 * \endcode
		 *
		 * For any instruction with \c A as the operand, e.g. <tt>OPC A</tt>, the operation will occur on <tt>A</tt>.
		 */
		uint8_t SYSTEM65CORE Addr_ACC(void); //!< Operand is the \c A register

		/**
		 * Instructions using absolute addressing contain a full 16-bit address to identify the target to read/write.
		 */
		uint16_t SYSTEM65CORE Addr_ABS(void); //!< Operand is a two-byte absolute address.

		/**
		 * The address to be accessed by an instruction using <tt>X</tt>
		 * register indexed absolute addressing is computed by taking the 16-bit
		 * address from the instruction and added the contents of the <tt>X</tt>
		 * register. For example if <tt>X</tt> contains <tt>$92</tt> then an
		 * <tt>STA $2000,X</tt> instruction will store the accumulator at
		 * <tt>$2092</tt> (e.g. <tt>$2000</tt> + <tt>$92</tt>).
		 */
		uint16_t SYSTEM65CORE Addr_ABX(void); //!< Operand is a two-byte absolute address indexed with X

		/**
		 * The <tt>Y</tt> register indexed absolute addressing mode is the same
		 * as \ref Addr_ABX only with the contents of the <tt>Y</tt> register
		 * added to the 16-bit address from the instruction.
		 */
		uint16_t SYSTEM65CORE Addr_ABY(void); //!< Operand is a two-byte absolute address indexed with Y

		/**
		 * Immediate addressing allows the programmer to directly specify an
		 * 8-bit constant within the instruction. It is indicated by a '#'
		 * symbol followed by a numeric expression.
		 */
		uint16_t SYSTEM65CORE Addr_IMM(void); //!< Operand is a one-byte value used directly

		uint16_t SYSTEM65CORE Addr_IND(void); //!< Indirect; <tt>OPC ($HHLL)</tt> operand is effective address; effective address is value of address

		/**
		 * Indexed indirect addressing is normally used in conjunction with a
		 * table of address held on zero page. The address of the table is taken
		 * from the instruction and the X register added to it (with zero page
		 * wrap around) to give the location of the least significant byte of
		 * the target address.
		 */
		uint16_t SYSTEM65CORE Addr_INX(void); //!< Indexed indirect; operand is the value stored as a pointer in the zeropage address indexed by X

		/**
		 * Indirect indexed addressing is the most common indirection mode used
		 * on the 6502. In this mode, the instruction contains the zeropage
		 * location of the least significant byte of a 16-bit address. The
		 * <tt>Y</tt> register is added to this address to generated the actual
		 * target address for the operation.
		 */
		uint16_t SYSTEM65CORE Addr_INY(void); //!< Indirect indexed; operand is the Y-indexed value stored as a pointer in the zeropage address.

		uint16_t SYSTEM65CORE Addr_REL(void); //!< Relative; <tt>OPC $BB</tt> branch target is PC + offest <tt>$BB</tt>; bit 7 (V?) signifies negative offset

		/**
		 * An instruction using zero page addressing mode has only an 8-bit
		 * address operand. This limits it to addressing only the first 256
		 * bytes of memory (e.g. \c $0000 to <tt>$00FF</tt>), where the most
		 * significant byte of the address is always zero. In zero page mode
		 * only the least significant byte of the address is held in the
		 * instruction making it shorter by one byte (important for space
		 * saving) and one less memory fetch during execution (important for
		 * speed).
		 *
		 * For an instruction <tt>OPC $LL</tt>, the operand represents the low
		 * byte of an address in the zeropage (<tt>$0000</tt>-<tt>$00FF</tt>) range.
		 */
		uint16_t SYSTEM65CORE Addr_ZPG(void); //!< Operand is a one-byte address in the zeropage.

		/**
		 * The address to be accessed by an instruction using indexed zero page
		 * addressing is calculated by taking the 8-bit zero page address from
		 * the instruction and adding the current value of the <tt>X</tt> register to
		 * it. For example if the <tt>X</tt> register contains <tt>$0F</tt> and the
		 * instruction <tt>LDA $80,X</tt> is executed then the accumulator will
		 * be loaded from <tt>$008F</tt> (<tt>$80</tt> + <tt>$0F</tt> => <tt>$8F</tt>).
		 *
		 * The low byte of the address calculation wraps around if the sum of
		 * the base address and the register exceeds <tt>$FF</tt>; for instance,
		 * <tt>LDA $FF,X</tt> with <tt>X</tt> having a value of <tt>$01</tt> will cause the
		 * effective address to be <tt>$0000</tt>.
		 */
		uint16_t SYSTEM65CORE Addr_ZPX(void); //!< Zeropage, X-indexed (<tt>$0000</tt> + <tt>X</tt>)

		/**
		 * The address to be accessed by an instruction using indexed zero page
		 * addressing is calculated by taking the 8-bit zero page address from
		 * the instruction and adding the current value of the <tt>Y</tt>
		 * register to it. See \ref Addr_ZPX() for more information.
		 *
		 * \note This mode is only used with the LDX and STX instructions.
		 */
		uint16_t SYSTEM65CORE Addr_ZPY(void); //!< Zeropage, Y-indexed (<tt>$0000</tt> + <tt>Y</tt>)
		/** @} */

		/** Runs a single instruction.
		 *
		 * When executing, this function will update \ref m_CycleCount with the
		 * new cycle count. When this function returns, it can immediately be
		 * called again to execute another instruction.
		 */
		void SYSTEM65CORE Dispatch(void);

		/** \defgroup system65helpers Execution helper functions
		 *
		 * These functions perform various helper tasks for execution, such as
		 * setting flags, transferring data, etc. When these functions are
		 * inlined, their performance is on par with function macros but retains
		 * the benefits of "real" functions.
		 * @{
		 */

		/** Sets the Zero/Negative flags
		 *
		 * If an instruction sets a register value, the register is usually
		 * tested to see whether it is negative or zero; in either case, the
		 * appropriate flag is set with this function.
		 */
		void SYSTEM65CORE Insn_Set_ZN_Flags(uint8_t reg);

		/** @} */

		/** \defgroup system65instructions CPU instructions
		 *
		 * These functions perform the actual modification of machine state that
		 * each instruction would normally do. Each reads the current byte at pc
		 * and acts accordingly.
		 * @{
		 */

		// Load/Store

		/**
		 * Flags affected:
		 * * Z: Set if A == 0
		 * * N: Set if the MSB of A is set
		 */
		void SYSTEM65CORE Insn_LDA(void); //!< Loads a value into A

		/**
		 * Flags affected:
		 * * Z: Set if X == 0
		 * * N: Set if the MSB of X is set
		 */
		void SYSTEM65CORE Insn_LDX(void); //!< Loads a value into X

		/**
		 * Flags affected:
		 * * Z: Set if Y == 0
		 * * N: Set if the MSB of X is set
		 */
		void SYSTEM65CORE Insn_LDY(void); //!< Loads a value into Y

		/**
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_STA(void); //!< Stores A into memory

		/**
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_STX(void); //!< Stores X into memory

		/**
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_STY(void); //!< Stores Y into memory

		// Register Transfer

		/** Flags affected:
		 * * Z: Set if X == 0
		 * * N: Set if MSB of X is set
		 */
		void SYSTEM65CORE Insn_TAX(void); //!< Copies the value of A into X

		/** Flags affected:
		 * * Z: Set if Y == 0
		 * * N: Set if MSB of Y is set
		 */
		void SYSTEM65CORE Insn_TAY(void); //!< Copies the value of A into Y

		/** Flags affected:
		 * * Z: Set if A == 0
		 * * N: Set if MSB of A is set
		 */
		void SYSTEM65CORE Insn_TXA(void); //!< Copies the value of X into A

		/** Flags affected:
		 * * Z: Set if A == 0
		 * * N: Set of MSB of A is set
		 */
		void SYSTEM65CORE Insn_TYA(void); //!< Copies the value of Y into A

		// Stack Operations

		/** Flags affected:
		 * * Z: Set if X == 0
		 * * N: Set if MSB of X is set
		 */
		void SYSTEM65CORE Insn_TSX(void); //!< Transfers S to X

		/** Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_TXS(void); //!< Transfers X to S

		/** Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_PHA(void); //!< Push A

		/** Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_PHP(void); //!< Push P

		/** Flags affected:
		 * * Z: Set if A == 0
		 * * N: Set if MSB of A is set
		 */
		void SYSTEM65CORE Insn_PLA(void); //!< Pull A

		/** Flags affected:
		 * * All
		 */
		void SYSTEM65CORE Insn_PLP(void); //!< Pull P

		// Logical Operations

		/**
		 * A logical AND is performed, bit by bit, on <tt>A</tt> using the
		 * contents of a byte of memory.
		 *
		 * Flags affected:
		 * * Z: Set if A == 0
		 * * N: Set if MSB of A is set
		 */
		void SYSTEM65CORE Insn_AND(void); //!< Bitwise AND

		/**
		 * An exclusive OR is performed, bit by bit, on <tt>A</tt> using the
		 * contents of a byte of memory.
		 *
		 * Flags affected:
		 * * Z: Set if A == 0
		 * * N: Set if MSB of A is set
		 */
		void SYSTEM65CORE Insn_EOR(void); //!< Exclusive OR

		/**
		 * An inclusive OR is performed, bit by bit, on <tt>A</tt> using the
		 * contents of a byte of memory.
		 *
		 * Flags affected:
		 * * Z: Set if A == 0
		 * * N: Set if MSB of A is set
		 */
		void SYSTEM65CORE Insn_ORA(void); //!< Logical Inclusive OR

		/**
		 * This instruction is used to test if one or more bits are set in a
		 * target memory location. The mask pattern in <tt>A</tt> is ANDed with
		 * the value in memory to set or clear the zero flag, but the result is
		 * not kept. Bits 7 and 6 of the value from memory are copied into the
		 * <tt>N</tt> and <tt>V</tt> flags.
		 *
		 * Flags affected:
		 * * Z: Set if A == 0
		 * * V: Set to bit 6 of the memory value
		 * * N: Set to bit 7 of the memory value
		 */
		void SYSTEM65CORE Insn_BIT(void); //!< Bit test

		/** @} */
};

#endif // SYSTEM65_HPP
