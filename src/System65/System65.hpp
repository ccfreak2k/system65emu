#ifndef SYSTEM65_HPP
#define SYSTEM65_HPP

// Standard libs
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <atomic>
#include <chrono>
#include <ctime>
#include <iostream>

// Class-related libs
#include <SFML/System/Mutex.hpp>

/** \file System65.hpp
 * Interface for the \ref System65 class.
 */

/** \class System65
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
 * Some future ideas:
 *
 * * Turn the memory[] array into a general "bus" and give pointer/size to
 *   devices that want memory, in addition to main memory being allocated out of
 *   the same pool. OOB access wouldn't be an issue anymore, since in the worst
 *   case a value is read/written from actual memory as the pointers are 16 bits
 *   anyway.
 *
 * * Extensions for memory management and/or vector computation. This is rather
 *   complicated by the fact that the 6502 lacks hardware multiplication
 *   support, instead relying on the programmer to code their own
 *   implementation. Perhaps instead we could add ATC (Add Triple with Carry)
 *   and STC (Subtract Triple with Carry) and leave the rest to the programmers.
 *
 * \author ccfreak2k
 *
 * \todo Increment the cycle count in each stage of execution (allows for
 * accurate cycle penalty for cross-page reads).
 *
 * \todo A pointer table might be faster than a switch for instruction decoding.
 *
 * \todo Make sure m_breakFlagSet is set/cleared when appropriate, and make sure
 * that the value is obeyed when pushing/pulling pf. Maybe use a gating method
 * like "Helper_PushFlags()"?
 *
 * \todo Handle the case where a second NMI is generated when one is already
 * being serviced. Do we stall it? Find some way to stack them? Mask it out?
 */

/** \def FASTEMU When defined, functions will be inlined for speed. May increase code size heavily.
 *
 * \note Best to leave this disabled for now, as gcc/msvc can only inline within
 * the translation unit unless LTO/LTCG is used.
 */
#ifdef FASTEMU
	#define SYSTEM65CORE inline
#else
	#define SYSTEM65CORE
#endif

#define STACK_BASE 0x0100 //!< Base address for the stack
#define CODE_BASE 0x0200 //!< Base address for code

/** Function macro to assert the correct decoding of an instruction */
#define ASSERT_INSN(byte) assert(memory[pc] == byte)

/** Function macro for writing a value from a register to memory */
#define INSN_R_TO_M(isize,ccount,reg,addrmode) \
	m_CycleCount += ccount; \
	memory[addrmode] = reg; \
	pc += isize

/** Function macro for writing a value from memory to a register */
#define INSN_M_TO_R(isize,ccount,reg,addrmode) \
	m_CycleCount += ccount; \
	reg = memory[addrmode]; \
	pc += isize

/** Function macro for copying a value from one register to another */
#define INSN_R_TO_R(isize,ccount,regdst,regsrc) \
	m_CycleCount += ccount; \
	regdst = regsrc; \
	pc += isize

/** Function macro for setting/clearing a flag based on a value. */
#define HELPER_SETCLEARFLAG(cond,pflag) \
	if (cond) \
		Helper_SetFlag(pflag); \
	else \
		Helper_ClearFlag(pflag)

/** Function macro for reporting an erroneous instruction decode */
#define INSN_DECODE_ERROR() \
	printf("ERROR: %s called with opcode 0x%.2X\n", __FUNCTION__, memory[pc])

/** \def PRINT_INSTRUCTION Prints the currently executing instruction */
#if defined(_MSC_VER) && !defined(__MINGW32__)
#define PRINT_INSTRUCTION() \
	printf("[DEBUG] %s, pc = 0x%.4X\n", __FUNCSIG__, pc)
#else
	printf("[DEBUG] %s, pc = 0x%.4X\n", __PRETTY_FUNCTION__, pc)
#endif // defined(_MSC_VER) && !defined(__MINGW32__)

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
		uint8_t GetRegister_A(void) { return a; };

		/** Returns the contents of the X index register. */
		uint8_t GetRegister_X(void) { return x; };

		/** Returns the contents of the Y index register. */
		uint8_t GetRegister_Y(void) { return y; };

		/** Returns the contents of the processor flags register. */
		uint8_t GetRegister_P(void) { return pf; };

		/** Returns the contents of the stack pointer register. */
		uint8_t GetRegister_S(void) { return s; };

		/** Returns the contents of the program counter register. */
		uint16_t GetRegister_PC(void) { return pc; };

		/** Returns the byte at the given address in the vm's memory.
		 *
		 * \param[in] addr Address to retrieve the byte from
		 *
		 * \return The byte value at the given address.
		 */
		uint8_t PeekByte(uint16_t addr) { return Helper_PeekByte(addr); }

		/** Returns the word at the given address in the vm's memory.
		 *
		 * \param[in] addr Address to retrieve the word from
		 *
		 * \return The word value at the given address.
		 *
		 * \note The 6502 uses little-endian format for multi-byte values; this
		 * method obeys that endianness, regardless of platform.
		 */
		uint16_t PeekWord(uint16_t addr) { return Helper_PeekWord(addr); }

		/** Writes <tt>val</tt> to memory at <tt>addr</tt>.
		 *
		 * \param[in] addr Address to write the value to
		 * \param[in] val Value to write to the given address
		 */
		void Poke(uint16_t addr, uint8_t val) { Helper_Poke(addr,val); }

		/** Writes <tt>val</tt> to memory at <tt>addr</tt>.
		 *
		 * \param[in] addr Address to write the value to
		 * \param[in] val Value to write to the given address
		 *
		 * \note The 6502 uses little-endian format for multi-byte values; this
		 * method obeys that endianness, regardless of platform.
		 */
		void Poke(uint16_t addr, uint16_t val) { Helper_Poke(addr,val); }

		/** Pushes a byte to the virtual CPU's stack
		 *
		 * \param[in] val Value to push onto the stack
		 */
		void Push(uint8_t val) { Helper_Push(val); }

		/** Pushes a word to the virtual CPU's stack
		*
		* \param[in] val Value to push onto the stack
		*
		* \note The 6502 uses little-endian format for multi-byte values; this
		* method obeys that endianness, regardless of platform.
		*/
		void Push(uint16_t val) { Helper_Push(val); }

		/** Pops a byte off of the virtual CPU's stack
		 *
		 * \return The byte at the top of the stack
		 */
		uint8_t PopByte(void) { return Helper_PopByte(); }

		/** Pops a word off of the virtual CPU's stack
		 *
		 * \return The word at the top of the stack
		 *
		 * \note The 6502 uses little-endian format for multi-byte values; this
		 * method obeys that endianness, regardless of platform.
		 */
		uint16_t PopWord(void) { return Helper_PopWord(); }

		/** Generates an external interrupt
		 *
		 * When called, an interrupt will be generated before the next
		 * instruction executes. A generated interrupt may be maskable (IRQ) or
		 * non-maskable (NMI). A maskable interrupt may be ignored if the I flag
		 * is set or if another interrupt is currently being handled; a
		 * non-maskable interrupt will always interrupt the CPU.
		 *
		 * \param[in] nmi Whether this generated interrupt should be maskable
		 * (IRQ) or non-maskable (NMI).
		 *
		 * \note Although the <tt>RESET</tt> signal is considered an interrupt
		 * from a technical standpoint, it is currently handled independent of
		 * this method.
		 *
		 * \see Reset
		 */
		void Interrupt(bool nmi);

		/** Sets the stack base page
		 *
		 * By default the stack page is 0x02 (effective start address is
		 * 0x02FF).
		 *
		 * \param[in] base New base page for the stack
		 */
		void SetStackBasePage(uint8_t base);

		/** Sets the interrupt vector address
		 *
		 * \param[in] ivec New address to vector to on interrupt
		 */
		void SetInterruptVector(uint16_t ivec);

		/** Resets the cycle counter. */
		void ResetCycleCount(void) { m_CycleCount = 0; }

		/** Loads a program into memory from another memory pointer
		 *
		 * This will load up to 64KB - 512 bytes (65023 bytes / 63.5KB) of
		 * program data pointed to by progmem into emulator memory. The function
		 * will return when either the maximum amount of data has been loaded or
		 * the limit specified by progsize has been reached.
		 *
		 * \note offset is currently not bounds-checked.
		 *
		 * \param[in] progmem Program data to load into emulator memory
		 * \param[in] progsize Size of the program data, in bytes
		 * \param[in] offset The address in memory that the binary should be
		 * loaded to.
		 */
		void LoadProgram(void *progmem, unsigned int progsize, unsigned int offset = 0x200);

		/** Loads a program into memory from a file
		 *
		 * This will load up to 64KB - 512 bytes (65023 bytes / 63.5KB) of
		 * program data from the file pointed to by progfile into emulator
		 * memory. The function will return when either the maximum amount of
		 * data has been loaded or the end of the file has been reached.
		 *
		 * \note offset is currently not bounds-checked.
		 *
		 * \param[in] progfile File handle to a file to load into emulator
		 * memory
		 * \param[in] offset The address in memory that the binary should be
		 * loaded to.
		 */
		void LoadProgram(FILE *progfile, unsigned int offset = 0x200);

		/** Performs a hot reset of the machine.
		 *
		 * PC and S are reset to their starting values; all other registers and
		 * flags are left untouched.
		 */
		void Reset(void);

		uint8_t *memory; //!< Pointer to system memory for this system
		unsigned int memorysize; //!< Size of memory for this system.

	protected:
	private:
		unsigned int m_CycleCount; //!< Tracks the number of cycles executed so far.

		unsigned int m_InstructionCount; //!< Tracks the number of instructions retired (executed) so far.

		std::clock_t m_CStart; //!< Starting clock for measuing execution speed
		std::clock_t m_CStop; //!< Ending clock for measuring execution speed

		uint16_t m_StackBase; //!< Base address that the stack resides at

		bool m_GenerateInterrupt; //!< Whether an interrupt is to be generated on the next tick \see Interrupt
		bool m_NMInterrupt; //!< Whether the generated interrupt is non-maskable \see m_Interrupt
		uint16_t m_InterruptVector; //!< Which address the next interrupt should vector to \see Interrupt

		/**
		 * The B flag is not an actual flag but is instead a bit set internally.
		 * What this means from a practical standpoint is that the B flag can't
		 * be cleared or set by code normally; instead, it's pushed as set when
		 * BRK or PHP is executed and pushed as clear when an IRQ or NMI occurs.
		 * Even after a BRK returns or if a byte is popped with 0x10 clear, the
		 * B flag will remain set, and will continue to be set unless an IRQ or
		 * NMI occurs (i.e. if an external interrupt never occurs, the B flag
		 * will remain set). The B flag is only meaningful within interrupt
		 * handlers anyway, so programmers don't need to worry too much about
		 * it.
		 */
		bool m_BreakFlagSet; //!< Indicates whether or not the B flag should be set whenever the flags are pushed or pulled.

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
			/** Carry flag
			 *
			 * The carry flag is set if the last operation caused an overflow
			 * from bit 7 of the result or an underflow from bit 0. This
			 * condition is set during arithmetic, comparison and during logical
			 * shifts. It can be explicitly set using the Set Carry Flag (SEC)
			 * instruction and cleared with Clear Carry Flag (CLC).
			 */
			PFLAG_C = 0x01,

			/** Zero flag
			 *
			 * The zero flag is set if the result of the last operation was zero.
			 */
			PFLAG_Z = 0x02,

			/** Interrupt Disable flag
			 *
			 * The interrupt disable flag is set if the program has executed a
			 * Set Interrupt Disable (SEI) instruction. While this flag is set
			 * the processor will not respond to interrupts from devices until
			 * it is cleared by a Clear Interrupt Disable (CLI) instruction.
			 */
			PFLAG_I = 0x04,

			/** Decimal Mode flag
			 *
			 * While the decimal mode flag is set the processor will obey the
			 * rules of Binary Coded Decimal (BCD) arithmetic during addition
			 * and subtraction. The flag can be explicitly set using Set Decimal
			 * Flag (SED) and cleared with Clear Decimal Flag (CLD).
			 */
			PFLAG_D = 0x08,

			/** Break flag
			 *
			 * The break command bit is set when a BRK instruction has been
			 * executed and an interrupt has been generated to process it.
			 */
			PFLAG_B = 0x10,

			PFLAG_R = 0x20, //!< Reserved (always 1)

			/** Overflow flag
			 *
			 * The overflow flag is set during arithmetic operations if the
			 * result has yielded an invalid 2's complement result (e.g. adding
			 * to positive numbers and ending up with a negative result: 64 + 64
			 * => -128). It is determined by looking at the carry between bits 6
			 * and 7 and between bit 7 and the carry flag.
			 */
			PFLAG_V = 0x40,

			/** Negative flag
			 *
			 * The negative flag is set if the result of the last operation had
			 * bit 7 set.
			 */
			PFLAG_N = 0x80
		};

		/** \defgroup addressmodes  Memory addressing modes
		 *
		 * These methods translate input address data and mode and return the
		 * actual address to read/write. Each specific opcode that reads from or
		 * writes to an address invokes one of these, but these functions read
		 * directly from the machine state.
		 *
		 * Unless otherwise noted, each function returns an \em index to the memory
		 * array as a \c uint8_t, which can be treated as an absolute pointer in
		 * memory.
		 *
		 * \note Each method returns 16-bit addresses in big-endian (MSB
		 * first) to index the memory array correctly.
		 *
		 * \note Most method descriptions in this group taken from this web
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
		 *
		 * \note This method is unused.
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
		 * <tt>$2092</tt> (i.e. <tt>$2000</tt> + <tt>$92</tt>).
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

		/**
		 * \ref Insn_JMP is the only 6502 instruction to support indirection.
		 * The instruction contains a 16-bit address which identifies the
		 * location of the least significant byte of another 16-bit memory
		 * address which is the real target of the instruction.
		 *
		 * For example if location <tt>$0120</tt> contains <tt>$FC</tt> and
		 * location <tt>$0121</tt> contains <tt>$BA</tt> then the instruction
		 * <tt>JMP ($0120)</tt> will cause the next instruction execution to
		 * occur at <tt>$BAFC</tt> (e.g. the contents of <tt>$0120</tt> and
		 * <tt>$0121</tt>).
		 */
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

		/**
		 * Relative addressing mode is used by branch instructions (e.g.
		 * \ref Insn_BEQ, \ref Insn_BNE, etc.) which contain a signed 8-bit relative
		 * offset (from -128 to +127) which is added to <tt>PC</tt> if the
		 * condition is true. As <tt>PC</tt> itself is incremented during
		 * instruction execution by two the effective address range for the
		 * target instruction must be with -126 to +129 bytes of the branch.
		 *
		 * \note This method is unused.
		 */
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

		/**
		 * If an instruction sets a register value, the register is usually
		 * tested to see whether it is negative or zero; in either case, the
		 * appropriate flag is set with this function.
		 */
		void SYSTEM65CORE Helper_Set_ZN_Flags(uint8_t reg); //!< Sets the Zero/Negative flags

		void SYSTEM65CORE Helper_Push(uint8_t val); //!< Push a single byte onto the stack

		void SYSTEM65CORE Helper_Push(uint16_t val); //!< Push a word (two bytes) onto the stack

		/**
		 * \return byte popped from the stack
		 */
		uint8_t SYSTEM65CORE Helper_PopByte(void); //!< Pop a single byte off of the stack

		/**
		 * \return word popped from the stack
		 */
		uint16_t SYSTEM65CORE Helper_PopWord(void); //!< Pop a word (two bytes) off of the stack

		/**
		 * A wrapper for most of the functionality for branch instructions.
		 *
		 * \param[in] branch Bool for whether or not to read the next byte for
		 * relative jump
		 */
		void SYSTEM65CORE Helper_SetBranch(bool branch); //!< Set PC to the relative address

		/**
		 * \param[in] addr Address to read the value from
		 *
		 * \return Value at the specified address
		 */
		uint8_t SYSTEM65CORE Helper_PeekByte(uint16_t addr); //!< Read a byte from memory

		/**
		 * \param[in] addr Address to read the value from
		 *
		 * \return Value at the specified address
		 */
		uint16_t SYSTEM65CORE Helper_PeekWord(uint16_t addr); //!< Read a word (2 bytes) from memory

		/**
		 * \param[in] addr Address to write the value to
		 * \param[in] val Value to write to the specified address
		 */
		void SYSTEM65CORE Helper_Poke(uint16_t addr, uint8_t val); //!< Write a byte into memory

		/**
		 * \param[in] addr Address to write the value to
		 * \param[in] val Value to write to the specified address
		 */
		void SYSTEM65CORE Helper_Poke(uint16_t addr, uint16_t val); //!< Write a word (2 bytes) into memory

		/**
		 * \param[in] flag Flag to set
		 */
		void SYSTEM65CORE Helper_SetFlag(System65::PFLAGS flag); //!< Sets a processor state flag

		/**
		 * \param[in] flag Flag to clear
		 */
		void SYSTEM65CORE Helper_ClearFlag(System65::PFLAGS flag); //!< Clears a processor state flag

		/**
		 * This method returns whether or not the given flag bit is set.
		 *
		 * \param[in] flag Flag to query the state of
		 *
		 * \return true if flag is set, false if flag is cleared
		 */
		bool SYSTEM65CORE Helper_GetFlag(System65::PFLAGS flag); //!< Queries the state of a flag

		/**
		 * This method sets or clears the C flag depending on the value of val.
		 *
		 * \param[in] val When true, the C flag is set; when false, the C flag is cleared.
		 */
		void SYSTEM65CORE Helper_SetClearC(bool val); //!< Sets or clears the C flag

		/**
		 * This method sets or clears the Z flag depending on the value of val.
		 *
		 * \param[in] val When true, the Z flag is set; when false, the Z flag is cleared.
		 */
		void SYSTEM65CORE Helper_SetClearZ(bool val); //!< Sets or clears the Z flag

		/**
		 * This method sets or clears the chosen flag depending on the value of val.
		 *
		 * \param[in] pflag Which flag to set or clear
		 * \param[in] val When true, the chosen flag is set; when false, the chosen flag is cleared.
		 */
		void SYSTEM65CORE Helper_SetClear(System65::PFLAGS pflag, bool val);

		/** Internal helper method for generating an interrupt
		 *
		 * This method generates an interrupt for the CPU core. Three of the
		 * four interrupts types are generated here (BRK, IRQ and NMI). The
		 * RESET interrupt is handled independently at the moment.
		 *
		 * It is an error for both nmi and sbrk to be set to TRUE.
		 *
		 * \param[in] nmi Whether this is a maskable or non-maskable interrupt.
		 * \param[in] sbrk Whether this is a a software (BRK) interrupt.
		 */
		void SYSTEM65CORE Helper_SetInterrupt(bool nmi, bool sbrk);

		/** Handles the pending interrupt condition
		 *
		 * The return value denotes whether the interrupt was serviced.
		 *
		 * \return Returns <tt>true</tt> if the interrupt was run, or
		 * <tt>false</tt> if the interrupt was skipped.
		 */
		bool SYSTEM65CORE Helper_HandleInterrupt(void);

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
		void SYSTEM65CORE Insn_PHA(void); //!< Pushes A to the stack

		/** Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_PHP(void); //!< Pushes P to the stack

		/** Flags affected:
		 * * Z: Set if A == 0
		 * * N: Set if MSB of A is set
		 */
		void SYSTEM65CORE Insn_PLA(void); //!< Pops from the stack to A

		/** Flags affected:
		 * * All
		 */
		void SYSTEM65CORE Insn_PLP(void); //!< Pops from the stack to P

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

		// Arithmetic Operations

		/**
		 * This instruction adds the contents of a memory location to <tt>A</tt>
		 * together with the carry bit. If overflow occurs the carry bit is set;
		 * this enables multiple byte addition to be performed.
		 *
		 * Flags affected:
		 * * C: Set if overflow in bit 7
		 * * Z: Set if A == 0
		 * * V: Set if sign bit is incorrect
		 * * N: Set if MSB of A is set
		 */
		void SYSTEM65CORE Insn_ADC(void); //!< Add with carry

		/**
		 * This instruction subtracts the contents of a memory location to
		 * <tt>A</tt> together with the not of the carry bit. If overflow occurs
		 * the carry bit is clear, this enables multiple byte subtraction to be
		 * performed.
		 *
		 * Flags affected:
		 * * C: Clear if overflow in bit 7
		 * * Z: Set if A == 0
		 * * V: Set if sign bit is incorrect
		 * * N: Set if MSB of A is set
		 */
		void SYSTEM65CORE Insn_SBC(void); //!< Subtract with carry

		/**
		 * This instruction compares the contents of the accumulator with
		 * another memory held value and sets the zero and carry flags as
		 * appropriate.
		 *
		 * Flags affected:
		 * * C: Set if A >= M
		 * * Z: Set if A == M
		 * * N: Set if MSB of result is set
		 */
		void SYSTEM65CORE Insn_CMP(void); //!< Compare accumulator

		/**
		 * This instruction compares the contents of <tt>X</tt> with another
		 * memory held value and sets the zero and carry flags as appropriate.
		 *
		 * Flags affected:
		 * * C: Set if X >= M
		 * * Z: Set if X == M
		 * * N: Set if MSB of result is set
		 */
		void SYSTEM65CORE Insn_CPX(void); //!< Compare X register

		/**
		 * This instruction compares the contents of <tt>Y</tt> with another
		 * memory held value and sets the zero and carry flags as appropriate.
		 *
		 * Flags affected:
		 * * C: Set if Y >= M
		 * * Z: Set if Y == M
		 * * N: Set if MSB of result is set
		 */
		void SYSTEM65CORE Insn_CPY(void); //!< Compare Y register

		// Increment/Decrement

		/**
		 * Adds one to the value held at a specified memory location setting the
		 * zero and negative flags as appropriate.
		 *
		 * Flags affected:
		 * * Z: Set if result is zero
		 * * N: Set if MSB of the result is set.
		 */
		void SYSTEM65CORE Insn_INC(void); //!< Increment a memory location

		/**
		 * Adds one to <tt>X</tt>, setting the zero and negative flags as
		 * appropriate.
		 *
		 * Flags affected:
		 * * Z: Set if X is zero
		 * * N: Set if MSB of X is set
		 */
		void SYSTEM65CORE Insn_INX(void); //!< Increment X register

		/**
		 * Adds one to <tt>Y</tt>, setting the zero and negative flags as
		 * appropriate.
		 *
		 * Flags affected:
		 * * Z: Set if Y is zero
		 * * N: Set if MSB of Y is set
		 */
		void SYSTEM65CORE Insn_INY(void); //!< Increment Y register

		/**
		 * Subtracts one from the value held at a specified memory location,
		 * setting the zero and negative flags as appropriate.
		 *
		 * Flags affected:
		 * * Z: Set if result is zero
		 * * N: Set if MSB of the result is set
		 */
		void SYSTEM65CORE Insn_DEC(void); //!< Decrement a memory location

		/**
		 * Subtracts one from <tt>X</tt>, setting the zero and negative flags as
		 * appropriate.
		 *
		 * Flags affected:
		 * * Z: Set if X is zero
		 * * N: Set if MSB of X is set
		 */
		void SYSTEM65CORE Insn_DEX(void); //!< Decrement X register

		/**
		 * Subtracts one from <tt>Y</tt>, setting the zero and negative flags as
		 * appropriate.
		 *
		 * Flags affected:
		 * * Z: Set if Y is zero
		 * * N: Set if MSB of Y is set.
		 */
		void SYSTEM65CORE Insn_DEY(void); //!< Decrement Y register

		// Shifts

		/**
		 * This operation shifts all the bits of the accumulator or memory
		 * contents one bit left. Bit 0 is set to 0 and bit 7 is placed in the
		 * carry flag. The effect of this operation is to multiply the memory
		 * contents by 2 (ignoring 2's complement considerations), setting the
		 * carry if the result will not fit in 8 bits.
		 *
		 * \todo Find out if Z is set on the memory location as well
		 * \todo Check if MSB/sign bit needs to be preserved
		 *
		 * Flags affected:
		 * * C: Set if MSB is set before shifting
		 * * Z: Set if result is zero
		 * * N: Set if MSB of result is set
		 */
		void SYSTEM65CORE Insn_ASL(void); //!< Arithmetic shift left

		/**
		 * Each of the bits in <tt>A</tt> or the specified memory location are
		 * shifted one place to the right. The bit that was in bit 0 is shifted
		 * into the carry flag. Bit 7 is set to zero.
		 *
		 * Flags affected:
		 * * C: Set to contents of LSB before shifting
		 * * Z: Set if result is zero
		 * * N: Set if MSB of result is set
		 */
		void SYSTEM65CORE Insn_LSR(void); //!< Logical shift right

		/**
		 * Move each of the bits in either <tt>A</tt> or the specified memory
		 * location one place to the left. Bit 0 is filled with the current
		 * value of the carry flag whilst the old bit 7 becomes the new carry
		 * flag value.
		 *
		 * \todo Check affected flags for correctness
		 *
		 * Flags affected:
		 * * C: Set to contents of MSB before shifting
		 * * Z: Set if result is 0
		 * * N: Set if MSB of result is set
		 */
		void SYSTEM65CORE Insn_ROL(void); //!< Rotate left

		/**
		 * Move each of the bits in either <tt>A</tt> or the specified memory
		 * location one place to the right. Bit 7 is filled with the current
		 * value of the carry flag whilst the old bit 0 becomes the new carry
		 * flag value.
		 *
		 * \todo Check affected flags for correctness
		 *
		 * Flags affected:
		 * * C: Set to contents of old bit 0
		 * * Z: Set if result is 0
		 * * N: Set if MSB of the result is set
		 */
		void SYSTEM65CORE Insn_ROR(void); //!< Rotate right

		// Jumps/Calls

		/**
		 * Sets PC to the address specified by the operand.
		 *
		 * \note On the original 6502, the indirect vector is not fetched
		 * correctly if it straddles a page boundary. Since this bug was fixed
		 * on many CMOS variants, this behavior is not implemented.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_JMP(void); //!< Jump to another location

		/**
		 * The <tt>JSR</tt> instruction pushes the address (minus one) of the
		 * return point on to the stack and then sets the program counter to the
		 * target memory address.
		 *
		 * \todo Does he mean "plus one?"
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_JSR(void); //!< Jump to a subroutine

		/**
		 * The <tt>RTS</tt> instruction is used at the end of a subroutine to
		 * return to the calling routine. It pulls the program counter (minus
		 * one) from the stack.
		 *
		 * \todo Does he mean "plus one?"
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_RTS(void); //!< Return from a subroutine

		// Branches

		/**
		 * If the carry flag is clear, then add the relative displacement to PC
		 * to cause a branch to a new location.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_BCC(void); //!< Branch if carry flag is clear

		/**
		 * If the carry flag is set, then add the relative displacement to PC to
		 * cause a branch to a new location.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_BCS(void); //!< Branch if carry flag is set

		/**
		 * If the zero flag is set, then add the relative displacement to PC to
		 * cause a branch to a new location.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_BEQ(void); //!< Branch if zero flag is set

		/**
		 * If the negative flag is set, then add the relative displacement to PC
		 * to cause a branch to a new location.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_BMI(void); //!< Branch if negative flag is set

		/**
		 * If the zero flag is clear, then add the relative displacement to PC
		 * to cause a branch to a new location.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_BNE(void); //!< Branch if zero flag is clear

		/**
		 * If the negative flag is clear, then add the relative displacement to
		 * PC to cause a branch to a new location.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_BPL(void); //!< Branch if negative flag is clear

		/**
		 * If the overflow flag is set, then add the relative displacement to PC
		 * to cause a branch to a new location.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_BVC(void); //!< Branch if overflow flag is clear

		/**
		 * If the overflow flag is set, then add the relative displacement to PC
		 * to cause a branch to a new location.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_BVS(void); //!< Branch if overflow flag is set

		// Status flag operations

		/**
		 * Flags affected:
		 * * C: Cleared
		 */
		void SYSTEM65CORE Insn_CLC(void); //!< Clear carry flag

		/**
		 * Flags affected:
		 * * D: Cleared
		 */
		void SYSTEM65CORE Insn_CLD(void); //!< Clear decimal mode flag

		/**
		 * Flags affected:
		 * * I: Cleared
		 */
		void SYSTEM65CORE Insn_CLI(void); //!< Clear interrupt disable flag

		/**
		 * Flags affected:
		 * * V: Cleared
		 */
		void SYSTEM65CORE Insn_CLV(void); //!< Clear overflow flag

		/**
		 * Flags affected:
		 * * C: Set
		 */
		void SYSTEM65CORE Insn_SEC(void); //!< Set carry flag

		/**
		 * Flags affected:
		 * * D: Set
		 */
		void SYSTEM65CORE Insn_SED(void); //!< Set decimal mode flag

		/**
		 * Flags affected:
		 * * I: Set
		 */
		void SYSTEM65CORE Insn_SEI(void); //!< Set interrupt disable flag

		// System operations

		/**
		 * The <tt>BRK</tt> instruction forces the generation of an interrupt
		 * request. <tt>PC</tt> and processor status are pushed on the stack,
		 * then the IRQ interrupt vector at <tt>$FFFE/<tt>$FFFF</tt> is loaded
		 * into <tt>PC</tt> and the B flag in the status register is set to 1.
		 *
		 * Flags affected:
		 * * B: Set to 1
		 */
		void SYSTEM65CORE Insn_BRK(void); //!< Force an interrupt

		/**
		 * The <tt>NOP</tt> instruction causes no changes to the processor state
		 * other than the normal incrementing of <tt>PC</tt> to the next
		 * instruction.
		 *
		 * Flags affected:
		 * * none
		 */
		void SYSTEM65CORE Insn_NOP(void); //!< No operation

		/**
		 * The <tt>RTI</tt> instruction is used at the end of an interrupt
		 * processing routine. It pulls the processor flags from the stack
		 * followed by <tt>PC</tt>.
		 */
		void SYSTEM65CORE Insn_RTI(void); //!< Return from interrupt

		/** @} */
};

#endif // SYSTEM65_HPP
