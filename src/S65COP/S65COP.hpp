#ifndef S65COP_HPP
#define S65COP_HPP

/** \file S65COP.hpp
 * Interface for the \ref S65COP class.
 */

/** Address from which the "run" command is read/written
 *
 * Setting this to any value other than 0x00 tells the coprocessor to being
 * executing the commands in the command buffer. When the coprocessor reaches
 * the end of execution, it will set this value to 0x00. If the value is set by
 * the CPU to 0x00 during COP execution, the execution will stop before the next
 * instruction is run; however, the currently executing instruction will
 * continue.
 */
#define COP_ADDR_EXECUTE 0xF0

/** Size of internal scratch memory, in bytes.
 *
 * As of this writing (2014-07-31) the size is 128 bytes (0x80).
 */
#define COP_SCRATCH_SIZE 0x80

/** @defgroup insnlist Instruction Listing
 * This is the listing of instructions currently supported for the System65
 * coprocessor module.
 *
 * Instructions may be grouped by their rough function. Currently:
 * * 0x00 are miscellaneous/status functions
 * * 0x10 perform loads from memory
 * * 0x20 perform stores to memory
 * * 0x30 are the four basic arithmetic operations
 * * 0x40 are the vector operations
 * @{
 */

typedef enum {
	/* Miscellanous/status/system */
	STP = 0x00, //!< Halts execution. IP is reset to 0.
	PSE = 0x01, //!< Halts execution. IP is not reset to 0.
	NOP = 0x02, //!< Does nothing for 1 tick.
	/* Loads */
	LIX = 0x10, //!< Loads an integer into IX.
	LIY = 0x11, //!< Loads an integer into IY.
	LIS = 0x12, //!< Loads an integer into scratch space.
	LVX = 0x18, //!< Loads a vector into VX.
	LVY = 0x19, //!< Loads a vector into VY.
	LVS = 0x1A, //!< Loads a vector into scratch space.
	/* Stores */
	SIX = 0x20, //!< Stores IX into the output buffer.
	SIY = 0x21, //!< Stores IY into the output buffer.
	SIS = 0x22, //!< Stores an integer from scratch space into the output buffer.
	SVX = 0x28, //!< Stores VX into the output buffer.
	SVY = 0x29, //!< Stores VY into the output buffer.
	SVS = 0x2A, //!< Stores a vector from scratch space into the output buffer.
	/* Basic math */
	AII = 0x30, //!< Add IY to IX; the result is stored in IX.
	AIV = 0x31, //!< Add IX to VX; the result is kept in VX.
	AVV = 0x32, //!< Add VY to VX; the result is stored in VX.
	SII = 0x34, //!< Subtract IY from IX; the result is stored in IX.
	SIV = 0x35, //!< Subtract IX from VX; the result is kept in VX.
	SVV = 0x36, //!< Subtract VY from VX; the result is stored in VX.
	MII = 0x38, //!< Multiply IX by IY; the result is stored in IX.
	MIV = 0x39, //!< Multiply VX by IX; the result is kept in VX.
	MVV = 0x3A, //!< Multiply VX by VY; the result is stored in VX.
	DII = 0x3C, //!< Divide IX by IY; the result is stored in IX.
	DIV = 0x3D, //!< Divide VX by IX; the result is kept in VX.
	DVV = 0x3E, //!< Divide VX by VY; the result is stored in VX.
	/* Vector ops */
	DOT = 0x40, //!< Dot product of VX and VY; the result is stored in VX.
	CRS = 0x41, //!< Cross product of VX and VY; the result is stored in VX.
	MID = 0x42, //!< The midpoint between VX and VY; the result is stored in VX.
} Opcode;

/** @} */

/** \class S65COP
 * The System65 Coprocessor class.
 *
 * This class represents the System65 Coprocessor. The coprocessor is used for
 * multiply/divide as well as vector operations.
 *
 * The coprocessor works by mapping a page of memory as a command buffer. The
 * processor puts commands into the buffer, and the coprocessor executes them in
 * order.
 */

/** \page workingdesc Coprocessor Description
 *
 * \section arch Coprocessor Architecture
 * The System65 Coprocessor is a 16-bit math coprocessor with support for
 * multiplication and division as well as operation on three-dimensional
 * vectors. It has the following features:
 *
 * * Two 16-bit integer registers: IX and IY
 * * Two 48-bit (3*16-bit) integer vector registers: VX and VY
 * * 128 bytes of internal scratch space (RAM) divided into 32 "slots" for
 * temporary storage of intermediate values
 * * Command buffer for sending data to the COP
 * * DMA access to system memory
 *
 * \section memdiv Memory Division
 *
 * The coprocessor memory space is 256 bytes in size and encompasses the
 * following:
 *
 * 0x00-0x7F (128 bytes): Command and immediate data buffer
 *
 * 0x80-0xEF (112 bytes): Result output storage
 *
 * 0xF0-0xFF (8 bytes): Command and status registers
 *
 * Additionally, the coprocessor has 128 bytes of internal "scratch" memory.
 * This can be used by code as temporary storage. The scratch space is divided
 * into 32 "slots" of 4 bytes, or 32 bits, each, enough to hold a single vector
 * value. Currently there is no way to pack values in the scratch space.
 *
 * \section futureideas Future Ideas
 *
 * Here are some potential ideas for the future:
 *
 * * Program storage and execution: Allow the CPU to upload small programs to
 * the COP and execute them on input data, which would allow for "custom"
 * instructions.
 * * Dispense with the scratch space and simply use main memory as the scratch
 * space. This would allow results to be dumped directly to RAM.
 * * Execution control instructions, such as conditionals.
 * * Instruction orthogonality; allowing each instruction to have several
 * * different ways of accepting data, much like 6502 instructions.
 */

class S65COP
{
	public:
		/** Initializes a coprocessor
		 *
		 * \param[in] base Base address for the command buffer
		 */
		S65COP(uint8_t *base);
		/** Uninitializes this coprocessor */
		~S65COP();

		/** Run one tick of the coprocessor. A tick may comprise more than one cycle. */
		void Tick(void);
	protected:
	private:
		/** Vector register typedef */
		typedef struct VectorRegister {
			uint16_t x; //!< X/1st component
			uint16_t y; //!< Y/2nd component
			uint16_t z; //!< Z/3rd component
		} VectorRegister;
		VectorRegister vx; //!< First vector register
		VectorRegister vy; //!< Second vector register
		uint16_t ix; //!< First integer register
		uint16_t iy; //!< Second integer register
		uint16_t ip; //!< Instruction pointer \FIXME Check if we actually need this
		uint8_t *scratch; //!< Internal RAM scratch space

		uint8_t *cmdbuf; //!< Command buffer page in system memory
		uint8_t  cmdptr; //!< Pointer within cmdbuf to the next data to read
};

#endif // S65COP_HPP
