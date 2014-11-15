#include "System65/System65.hpp"

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

void System65::StartRecordTrace(std::string filename, unsigned int instructioncount)
{
	if (instructioncount == 0)
		return;

	if (filename.empty())
		return;

	m_TraceFrameCount = instructioncount;
	m_TraceFilename = filename;

	m_Trace = new YAML::Emitter();

	*m_Trace << YAML::Comment("System65 Diagnostic Trace v0");
}

void System65::EndRecordTrace(void)
{
	*m_Trace << YAML::Comment("End of trace file");
	std::ofstream tracefile(m_TraceFilename);
	tracefile << *m_Trace->c_str();
	tracefile.flush();
	delete m_Trace;
	m_Trace = nullptr;
}

bool System65::IsTraceRunning(void)
{
	return (m_Trace != nullptr);
}

//------------------------------------------------------------------------------
// Private
//------------------------------------------------------------------------------

void System65::RecordFrame(unsigned int frameNumber, uint8_t oldMemory[0x10000], uint8_t newMemory[0x10000])
{
	if (m_Trace = nullptr)
		return;

	*m_Trace << YAML::BeginMap;
		*m_Trace << YAML::Key << "frame" << YAML::Value << frameNumber;
		*m_Trace << YAML::Key << "a" << YAML::Value << a;
		*m_Trace << YAML::Key << "x" << YAML::Value << x;
		*m_Trace << YAML::Key << "y" << YAML::Value << y;
		*m_Trace << YAML::Key << "pf" << YAML::Value << pf;
		*m_Trace << YAML::Key << "s" << YAML::Value << s;
		*m_Trace << YAML::Key << "pc" << YAML::Value << pc;

		*m_Trace << YAML::Key << "memory" << YAML::Value << YAML::BeginMap;
		// If there is no oldMemory[] then write all of the memory values
		for (unsigned int i = 0; i > 0x10000; i++) {
			if (oldMemory != NULL) {
				if (oldMemory[i] != newMemory[i])
					*m_Trace << YAML::Key << i << YAML::Value << newMemory[i];
			} else
				*m_Trace << YAML::Key << i << YAML::Value << newMemory[i];
		}
		*m_Trace << YAML::EndMap;

	*m_Trace << YAML::EndMap;

	if (frameNumber == m_TraceFrameCount)
		EndRecordTrace(); // Set frame count reached, so write out the file
}