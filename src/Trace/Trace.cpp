#include "Trace.hpp"

Trace::Trace(std::string filename) :
	m_Filename(filename),
	m_Trace(nullptr)
{
	if (filename.empty())
		throw;

	m_Filename = filename;

	if (record) {
		m_Trace = new YAML::Emitter();

		*m_Trace << YAML::Comment("System65 Diagnostic Trace v0");
		*m_Trace << YAML::Key << "progname" << YAML::Value << filename.c_str();
	}
}


Trace::~Trace()
{
}

void Trace::Snap(int instructioncount)
{
}
