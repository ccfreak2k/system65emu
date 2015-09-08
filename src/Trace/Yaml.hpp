#pragma once

// yaml_cpp needs this
#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#endif // _MSC_VER

// Standard libs
#include <fstream>
#include <memory>
#include <string>
#include <vector>

// Class-related libs
#include <yaml-cpp/yaml.h>

namespace Trace {
	/** \class Yaml
	* The System65 CPU emulator execution tracing class, utilizing YAML for
	* serialization.
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
	class Yaml
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
		Yaml(std::string filename = "trace");

		/** Terminates the tracing session.
		*/
		~Yaml();

		/** Traces through a single step of execution.
		*
		* This method records the machine state, indexed on
		* <tt>instructioncount</tt>. The caller should provide callback pointers to
		* methods that return the necessary information about the current machine state.
		*
		* \param[in] instructioncount The number of instructions that have been
		* executed up to this point.
		* \param[in] acb Callback function pointer for reading the contents of the accumulator
		* \param[in] xcb Callback function pointer for reading the contents of the X index register
		* \param[in] ycb Callback function pointer for reading the contents of the Y index register
		* \param[in] pcb Callback function pointer for reading the contents of the processor flags register
		* \param[in] scb Callback function pointer for reading the contents of the stack pointer register
		* \param[in] pccb Callback function pointer for reading the contents of the program counter register
		* \param[in] memcb Callback function pointer for reading the VM's memory
		*/
		void Snap(int instructioncount, uint8_t(*acb)(), uint8_t(*xcb)(), uint8_t(*ycb)(), uint8_t(*pcb)(), uint8_t(*scb)(), uint16_t(*pccb)(), uint8_t(*memcb)(uint16_t));

	protected:
	private:
		std::string m_Filename; //!< Name of the trace file to record to

		std::unique_ptr<YAML::Emitter> m_Trace; //!< YAML emitter to serialize the recorded data

		std::vector<uint8_t> m_OldMemory; //!< Vector object representing machine memory from the last call to Snap().

		std::ofstream m_File; //!< Output file object to write the data into

		bool m_FirstWrite; //!< Bool representing whether this is the first frame of data, i.e. if we need to dump the entire memory space
	};
}