#pragma once

// yaml_cpp needs this
#ifdef _MSC_VER
	#define _SCL_SECURE_NO_WARNINGS
#endif // _MSC_VER

// Standard libs
#include <string>

// Class-related libs
#include <yaml-cpp/yaml.h>

/** \class Trace
* The System65 CPU emulator execution tracing class.
*
* This class manages the tracing of execution in the System65 emulator. Its main
* use is in verifying the correctness of emulator changes. It snapshots the
* state of emulation on each stop point during the execution of the emulator
* (per-instruction granularity when using the reference \ref System65 class),
* serializes the trace data and allows playback comparison later.
*
* Recording and playback is delegated to the \ref TraceRecord and
* \ref TracePlayback classes, respectively.
*/

class Trace
{
public:
	/** Initializes the tracing system.
	 *
	 * This method starts the tracing system, allowing trace recording/playback
	 * to occur. Tracing is done to/from <tt>filename</tt>, with <tt>record</tt>
	 * to specify whether this object should record or playback the trace.
	 *
	 * \param[in] filename File to record to/playback from; an extension is not
	 * needed.
	 * \param[in] record Whether the trace should be recorded (<tt>true</tt>) or
	 * played back (<tt>false</tt>)
	 *
	 * \throws Passes any throws related to file handling.
	 */
	Trace(std::string filename = "trace");

	/** Terminates the tracing session. 
	 */
	~Trace();

	/** Traces through a single step of execution.
	 *
	 * This method records or plays back*/
	void Snap(int instructioncount);

protected:
	std::string m_Filename; //!< Filename for the trace session

private:
};

