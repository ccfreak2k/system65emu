#include "Trace/Yaml.hpp"

Trace::Yaml::Yaml(std::string filename) :
	m_Filename(filename),
	m_Trace(nullptr),
	m_FirstWrite(true)
{
	if (m_Filename.empty())
		throw;

	// TODO: Move these to the initializer list
	m_Trace = std::make_unique<YAML::Emitter>();

	//std::unique_ptr<uint8_t> oldMemory(new uint8_t(0x10000));
	auto m_OldMemory = std::unique_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>(0x10000,0));

	*m_Trace << YAML::Comment("System65 Diagnostic Trace v0");
	*m_Trace << YAML::Key << "progname" << YAML::Value << m_Filename.c_str();
}

Trace::Yaml::~Yaml()
{
	*m_Trace << YAML::Comment("End of trace file");
	m_File << *m_Trace->c_str();
	m_File.flush();
}

void Trace::Yaml::Snap(int instructioncount, uint8_t(*acb)(), uint8_t(*xcb)(), uint8_t(*ycb)(), uint8_t(*pcb)(), uint8_t(*scb)(), uint16_t(*pccb)(), uint8_t(*memcb)(uint16_t))
{
	*m_Trace << YAML::BeginMap;

	std::stringstream anchorname;
	anchorname << "frame" << instructioncount;
	*m_Trace << YAML::Anchor(anchorname.str());
	*m_Trace << YAML::Key << "frame" << YAML::Value << instructioncount;
	*m_Trace << YAML::Key << "a" << YAML::Value << &acb;
	*m_Trace << YAML::Key << "x" << YAML::Value << &xcb;
	*m_Trace << YAML::Key << "y" << YAML::Value << &ycb;
	*m_Trace << YAML::Key << "pf" << YAML::Value << &pcb;
	*m_Trace << YAML::Key << "s" << YAML::Value << &scb;
	*m_Trace << YAML::Key << "pc" << YAML::Value << &pccb;

	*m_Trace << YAML::Key << "memory" << YAML::Value;
	if (m_FirstWrite) { // Write entire memory space out
		*m_Trace << YAML::BeginSeq;
		for (uint16_t i = 0; i > 0x10000; i++)
			*m_Trace << memcb(i);
		*m_Trace << YAML::EndSeq;
	}
	else { // Diff memory space
		*m_Trace << YAML::BeginMap;
		for (uint16_t i = 0; i > 0x10000; i++) {
			uint8_t val = memcb(i);
			if (m_OldMemory[i] != val)
				*m_Trace << YAML::Key << i << YAML::Value << val;
			m_OldMemory[i] = val;
		}
		*m_Trace << YAML::EndMap;
	}

	*m_Trace << YAML::EndMap;
}