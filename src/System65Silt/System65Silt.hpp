#ifndef SYSTEM65SILT_CPP
#define SYSTEM65SILT_CPP

#include <windows.h>

// Standard libs
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <map>
#include <string>

/** \file System65Silt.hpp
 * Interface for the \ref System65Silt class.
 */

/** \class System65Silt
 * The System65 CPU emulator and dynamic recompiler class.
 *
 * This class implements the System65 emulator as the \ref System65 class does,
 * but it does so using what I call Simple Instruction-Level Translation, or
 * SILT. SILT takes each 6502 instruction and emits an equivalent single or set
 * of x86 instructions. The implementation was pulled from this article for
 * starters:
 *
 * http://web.archive.org/web/20121028174315/http://www.altdevblogaday.com/2011/06/12/jit-cpu-emulation-a-6502-to-x86-dynamic-recompiler-part-1
 *
 * So based on this, here's the projected layout in registers:
 * a  <-> AL
 * sp <-> AH
 * x  <-> BL
 * y  <-> BH
 * p  <-> CL
 * ip <-> DI
 */

class System65Silt
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
	System65Silt(unsigned int memsize = 0x1000, uint8_t stackbase = 0x02);

	/** Destroys the System65 context
		*/
	~System65Silt();

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
		* \note Because the dynarec will run multiple instructions at once, it
		* is difficult to run an exact number of emulated cycles each time.
		*/
	void Tick(unsigned int cycleLimit);

	/** Returns the contents of the accumulator register. */
	uint8_t GetRegister_A(void) { return m_Register.a; }

	/** Returns the contents of the X index register. */
	uint8_t GetRegister_X(void) { return m_Register.x; }

	/** Returns the contents of the Y index register. */
	uint8_t GetRegister_Y(void) { return m_Register.y; }

	/** Returns the contents of the processor flags register. */
	uint8_t GetRegister_P(void) { return m_Register.p; }

	/** Returns the contents of the stack pointer register. */
	uint8_t GetRegister_S(void) { return m_Register.s; }

	/** Returns the contents of the program counter register. */
	uint16_t GetRegister_PC(void) { return m_Register.pc; }

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
		* \param[in] newbase New base page for the stack
		*
		* \return The previous stack base page
		*/
	uint8_t SetStackBasePage(uint8_t newbase);

	/** Sets the interrupt vector address
		*
		* \param[in] ivec New address to vector to on interrupt
		*/
	void SetInterruptVector(uint16_t ivec);

	/** Begins recording the machine state to a trace file.
		*
		* The trace will record up to <tt>instructioncount</tt> number of
		* "frames" of trace data. If you are unsure of how many frames you'll need,
		* you can leave the number parameter out and call EndRecordTrace() when
		* you finish.
		*
		* \param[in] filename Path to the file to record the trace to; an empty
		* filename will return immediately.
		* \param[in] instructioncount Number of instructions to record to the
		* trace file; a value of 0 will return without creating a trace file.
		*/
	void StartRecordTrace(std::string filename = "trace.yml", unsigned int instructioncount = UINT_MAX);

	/** Ends a currently-recording trace.
		*
		*/
	void EndRecordTrace(void);

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

	/** Returns whether a trace is currently running
		*
		* \return Whether a trace is currently running
		*/
	bool IsTraceRunning(void);

	unsigned int memorysize; //!< Size of memory for this system.
protected:
private:
	/** Structure of CPU state.
	 */
	struct Registers {
		uint8_t a; //!< Accumulator
		uint8_t x; //!< X register
		uint8_t y; //!< Y register
		uint8_t p; //!< Processor flags register
		uint8_t s; //!< Stack pointer
		uint16_t pc; //!< Program counter (IP)
	};

	Registers m_Register; //!< Registers for this emulated CPU

	/** Structure representing a chunk of native translated code.
	 */
	struct NativeCode {
		uint8_t *ptr; //!< Pointer to a buffer containing the native code
		int size; //!< Size of the buffer for the native code
		int cyclecount; //!< Number of emulated instructions in this code block
	};

	uint8_t *m_Memory; //!< Pointer to system memory for this system

	uint16_t m_StackBase; //!< Address that the stack is based at.

	typedef std::map<uint16_t, NativeCode> CacheMap; //!< Typedef for the native code cache map
	CacheMap m_Cache; //!< The native code cache map declaration

	/** Operator for accessing memory like an array
	 */
	uint8_t &operator[](uint16_t offs)
	{
		return m_Memory[offs];
	}

	/** Translates the target code into native and caches the result.
	 */
	const NativeCode *Compile(uint16_t iptr);

	/** Performs the actual recompilation of code
	 */
	int CompileBlock(const uint8_t *in, uint8_t *out);

	/** Runs the cached native code.
	 *
	 * \note Throws if the cache doesn't have a matching code block.
	 */
	void Execute(const NativeCode *code);

	/**
	 * \param[in] addr Address to read the value from
	 *
	 * \return Value at the specified address
	 */
	uint8_t Helper_PeekByte(uint16_t addr); //!< Read a byte from memory

	/**
	 * \param[in] addr Address to read the value from
	 *
	 * \return Value at the specified address
	 */
	uint16_t Helper_PeekWord(uint16_t addr); //!< Read a word (2 bytes) from memory

	/**
	 * \param[in] addr Address to write the value to
	 * \param[in] val Value to write to the specified address
	 */
	void Helper_Poke(uint16_t addr, uint8_t val); //!< Write a byte into memory

	/**
	 * \param[in] addr Address to write the value to
	 * \param[in] val Value to write to the specified address
	 */
	void Helper_Poke(uint16_t addr, uint16_t val); //!< Write a word (2 bytes) into memory

	void Helper_Push(uint8_t val); //!< Push a single byte onto the stack

	void Helper_Push(uint16_t val); //!< Push a word (two bytes) onto the stack

	/**
	 * \return byte popped from the stack
	 */
	uint8_t Helper_PopByte(void); //!< Pop a single byte off of the stack

	/**
	 * \return word popped from the stack
	 */
	uint16_t Helper_PopWord(void); //!< Pop a word (two bytes) off of the stack

	//--------------------------------------------------------------------------

	/** \defgroup module_addressmodes Memory addressing modes
		*
		* These methods translate input address data and mode and return the
		* actual address to read/write. Each specific opcode that reads from or
		* writes to an address invokes one of these, but these functions read
		* directly from the machine state.
		*
		* Unless otherwise noted, each function returns an \em index to the memory
		* array as a \c uint16_t, which can be treated as an absolute pointer in
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
		* Instructions using absolute addressing contain a full 16-bit address to identify the target to read/write.
		*/
	uint16_t Addr_ABS(uint8_t *in); //!< Operand is a two-byte absolute address.

	/**
		* The address to be accessed by an instruction using <tt>X</tt>
		* register indexed absolute addressing is computed by taking the 16-bit
		* address from the instruction and added the contents of the <tt>X</tt>
		* register. For example if <tt>X</tt> contains <tt>$92</tt> then an
		* <tt>STA $2000,X</tt> instruction will store the accumulator at
		* <tt>$2092</tt> (i.e. <tt>$2000</tt> + <tt>$92</tt>).
		*/
	uint16_t Addr_ABX(uint8_t *in); //!< Operand is a two-byte absolute address indexed with X

	/**
		* The <tt>Y</tt> register indexed absolute addressing mode is the same
		* as \ref Addr_ABX only with the contents of the <tt>Y</tt> register
		* added to the 16-bit address from the instruction.
		*/
	uint16_t Addr_ABY(uint8_t *in); //!< Operand is a two-byte absolute address indexed with Y

	/**
		* Immediate addressing allows the programmer to directly specify an
		* 8-bit constant within the instruction. It is indicated by a '#'
		* symbol followed by a numeric expression.
		*/
	uint16_t Addr_IMM(uint8_t *in); //!< Operand is a one-byte value used directly

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
	uint16_t Addr_IND(uint8_t *in); //!< Indirect; <tt>OPC ($HHLL)</tt> operand is effective address; effective address is value of address

	/**
		* Indexed indirect addressing is normally used in conjunction with a
		* table of address held on zero page. The address of the table is taken
		* from the instruction and the X register added to it (with zero page
		* wrap around) to give the location of the least significant byte of
		* the target address.
		*/
	uint16_t Addr_INX(uint8_t *in); //!< Indexed indirect; operand is the value stored as a pointer in the zeropage address indexed by X

	/**
		* Indirect indexed addressing is the most common indirection mode used
		* on the 6502. In this mode, the instruction contains the zeropage
		* location of the least significant byte of a 16-bit address. The
		* <tt>Y</tt> register is added to this address to generated the actual
		* target address for the operation.
		*/
	uint16_t Addr_INY(uint8_t *in); //!< Indirect indexed; operand is the Y-indexed value stored as a pointer in the zeropage address.

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
	uint16_t Addr_REL(uint8_t *in); //!< Relative; <tt>OPC $BB</tt> branch target is PC + offest <tt>$BB</tt>; bit 7 (V?) signifies negative offset

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
	uint16_t Addr_ZPG(uint8_t *in); //!< Operand is a one-byte address in the zeropage.

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
	uint16_t Addr_ZPX(uint8_t *in); //!< Zeropage, X-indexed (<tt>$0000</tt> + <tt>X</tt>)

	/**
		* The address to be accessed by an instruction using indexed zero page
		* addressing is calculated by taking the 8-bit zero page address from
		* the instruction and adding the current value of the <tt>Y</tt>
		* register to it. See \ref Addr_ZPX() for more information.
		*
		* \note This mode is only used with the LDX and STX instructions.
		*/
	uint16_t Addr_ZPY(uint8_t *in); //!< Zeropage, Y-indexed (<tt>$0000</tt> + <tt>Y</tt>)
	/** @} */

	//--------------------------------------------------------------------------
	// NOP
	int i_nop(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);

	// JMP
	int i_jmpabs(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
	int i_jmpind(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);

	// JSR
	int i_jsrabs(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);

	// LDA
	int i_ldaimm(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
	int i_ldazpg(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
	int i_ldazpx(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
	int i_ldaabs(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
	int i_ldaabx(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
	int i_ldaaby(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
	int i_ldainx(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
	int i_ldainy(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);

	// LDX
	int i_ldximm(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);

	// LDY
	int i_ldyimm(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);

	// STA
	int i_staabs(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
	int i_staabx(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);
};

#endif // SYSTEM65SILT_CPP