#pragma once

// Standard libs
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace Trace {
	/** \class BinaryRecord
	* The System65 CPU emulator execution tracing class, utilizing binary output
	* for serialization.
	*
	* This class manages the tracing of execution in the System65 emulator. Its main
	* use is in verifying the correctness of emulator changes. It snapshots the
	* state of emulation on each stop point during the execution of the emulator
	* (per-instruction granularity when using the reference \ref System65 class),
	* serializes the trace data and allows playback comparison later.
	*
	* It is possible to record a complete snapshot of the machine state in a single
	* trace file by initializing and calling \ref Snap() at one point.
	*
	* This class serializes trace data by using a custom binary format. The goal
	* of the binary format is to compact data as much as possible in order to
	* minimize the disk footprint.
	*
	* \todo Get TraceRecord and TracePlayback to derive from the same base; the
	* constructor and destructors can be part of the base, as well as a few of the
	* members, but Snap() would be implemented independently because playback
	* returns a bool, unless I set Snap() in record to also have the bool type but
	* just not return anything.
	*
	* \todo Figure out the best approach for passing data to Snap(). Do I pass the
	* System65 object? Do I have a separate method for setting callbacks and require
	* getters? Do I just pass all 9 arguments every time?
	*/
	class BinaryRecord
	{
	public:
		/** Initializes the trace recording.
		*
		* This method starts the tracing system, allowing trace recording to occur.
		* Tracing is done to <tt>filename</tt>.
		*
		* \param[in] filename File to record to; an extension is not needed.
		*
		* \throws Passes any throws related to file handling.
		*/
		BinaryRecord(std::string filename = "trace");

		/** Terminates the tracing session.
		*/
		~BinaryRecord();

		/** Traces through a single step of execution.
		*
		* This method records the machine state, indexed on
		* <tt>instructioncount</tt>. The caller should provide callback pointers to
		* methods that return the necessary information about the current machine state.
		*
		* \param[in] instructioncount The number of instructions that have been
		* executed up to this point.
		* \param[in] a Current value of the accumulator
		* \param[in] x Current value of the X index register
		* \param[in] y Current value of the Y index register
		* \param[in] p Current value of the processor flags register
		* \param[in] s Current value of the stack pointer
		* \param[in] pc Current value of the program counter
		* \param[in] memcb Callback function pointer for reading the VM's memory
		*/
		void Snap(uint32_t instructioncount, uint8_t a, uint8_t x, uint8_t y, uint8_t p, uint8_t s, uint16_t pc, std::shared_ptr<std::vector<uint8_t>> mem);

	protected:
	private:
		const uint8_t m_FileMajorVersion = 0x00; //!< Major version of the binary file format
		const uint8_t m_FileMinorVersion = 0x00; //!< Minor version of the binary file format

		std::string m_Filename; //!< Filename of the trace file

		std::unique_ptr<std::ofstream> m_File; //!< Output file for the trace stream

		/** Struct representing the old state of the CPU (state on the previous recorded frame). */
		struct {
			uint8_t a;
			uint8_t x;
			uint8_t y;
			uint8_t p;
			uint8_t s;
			uint16_t pc;
		} m_OldState;

		std::unique_ptr<std::vector<uint8_t>> m_OldMemory; //!< Snapshot of memory from the last call of Snap().
	};
}