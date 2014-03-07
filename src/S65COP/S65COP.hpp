#ifndef S65COP_HPP
#define S65COP_HPP

/** \file S65COP.hpp
 * Interface for the \ref S65COP class.
 */

/** Address from which the "run" command is read/written
 *
 * Setting this to !0x00 tells the coprocessor to being executing the commands
 * in the command buffer. When the coprocessor reaches the end, it will set this
 * value to 0x00. If the value is set by the CPU to 0x00 during COP execution,
 * the execution will stop before the next instruction is run.
 */
#define ADDR_EXECUTE 0xF0

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
 * \section insns Coprocessor Instructions
 * The System65 Coprocessor currently supports the following instructions:
 *
 * * LIX: Load an integer into IX
 * * LIY: Load an integer into IY
 * * LIS: Load an integer into scratch space
 * * LVX: Load a vector into VX
 * * LVY: Load a vector into VY
 * * LVS: Load a vector into scratch space
 * * SIX: Store IX into the output buffer
 * * SIY: Store IY into the output buffer
 * * SIS: Store an integer from scratch space into the output buffer
 * * SVX: Store VX into the output buffer
 * * SVY: Store VY into the output buffer
 * * SVS: Store a vector from scratch space into the output buffer
 * * AII/AIV/AVV: Add integers and vectors
 * * SII/SIV/SVV: Subtract integers and vectors
 * * MII/MIV/MVV: Multiply integers and vectors
 * * DII/DIV/DVV: Divide integers and vectors
 * * DOT: Dot product of two vectors
 * * CRS: Cross product of two vectors
 * * MID: Calculates the midpoint of two vectors
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
 */

class S65COP
{
	public:
		/** Initializes a coprocessor
		 *
		 * \param[in] membus Pointer to the system memory bus
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
		uint8_t *scratch; //!< Internal RAM scratch space

		uint8_t *cmdbuf; //!< Command buffer page in system memory
		uint8_t  cmdptr; //!< Pointer within cmdbuf to the next data to read
};

#endif // S65COP_HPP
