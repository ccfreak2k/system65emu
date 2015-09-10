#include "Trace/BinaryRecord.hpp"

Trace::BinaryRecord::BinaryRecord(std::string filename) :
	m_Filename(filename)
{
	// Throw if the filename is blank
	if (m_Filename.empty())
		throw;

	// Allocate the memory snapshot
	m_OldMemory = std::make_unique<std::vector<uint8_t>>(0x10000, 0x00);

	// Append the extension and open the file
	std::string appendedfilename;
	appendedfilename = m_Filename;
	appendedfilename.append(".btr");
	m_File = std::make_unique<std::ofstream>(appendedfilename, std::ios::binary | std::ios::trunc);

	if (!m_File->good())
		throw;

	*m_File << m_FileMajorVersion << m_FileMinorVersion;

	if (!m_File->good())
		throw;
}

Trace::BinaryRecord::~BinaryRecord()
{
	m_File->close();
}

// TODO: Write the "native" values to the file instead of stuffing them into a buffer.
void Trace::BinaryRecord::Snap(uint32_t instructioncount, uint8_t a, uint8_t x, uint8_t y, uint8_t p, uint8_t s, uint16_t pc, std::shared_ptr<std::vector<uint8_t>> mem)
{
	if (!m_File->good())
		throw; // TODO: throw something specific

	// Format:
	// 0x0000: 4 byte: frame number (instruction count)
	// 0x0004: 2 byte: length (not including frame number)
	// cpu state:
	//  1 byte: identifier for register(s):
	//   0x01: a
	//   0x02: x
	//   0x04: y
	//   0x08: p
	//   0x10: s
	//   0x20: pc
	//   ? bytes: list of changed values for registers, in above order
	// list of changed memory locations:
	//  2 byte: address
	//  1 byte: value
	//
	// What we end up with is something like:
	// 0x00 0x00 0x00 0x01 0x00 0x08 0x01 0x3F 0x00 0x00 0xFF 0x00 0x01 0x01
	//
	// The above is example data, which is:
	// frame 1, register a has value 0x3F now, address 0x0000 has 0xFF and 0x0001 has 0x01.

	std::vector<uint8_t> buf(196622); // maximum frame size, assuming 14 + (65536*3) bytes per frame
	unsigned len = 0; // length of the frame

	uint8_t RegsModified = 0; // Which registers have been modified

	// We build the buffer as we go
	//HACK: more little endian writing
	buf[0] = instructioncount&0xff;
	buf[1] = (instructioncount >> 8) & 0xff;
	buf[2] = (instructioncount >> 16) & 0xff;
	buf[3] = (instructioncount >> 24) & 0xff;
	len += 7; // incrementing the length on each write

	// Compare registers
	if (a != m_OldState.a) {
		RegsModified |= 0b00000001;
		buf[len]= a;
		len++;
		m_OldState.a = a;
	}
	if (a != m_OldState.x) {
		RegsModified |= 0b00000010;
		buf[len] = x;
		len++;
		m_OldState.x = x;
	}
	if (a != m_OldState.y) {
		RegsModified |= 0b00000100;
		buf[len] = y;
		len++;
		m_OldState.y = y;
	}
	if (a != m_OldState.p) {
		RegsModified |= 0b00001000;
		buf[len] = p;
		len++;
		m_OldState.p = p;
	}
	if (a != m_OldState.s) {
		RegsModified |= 0b00010000;
		buf[len] = s;
		len++;
		m_OldState.s = s;
	}
	if (a != m_OldState.pc) {
		RegsModified |= 0b00100000;
		buf[len] = (pc&0xff); // more little endian writing
		buf[len + 1] = ((pc >> 8) & 0xff);
		len += 2;
		m_OldState.pc = pc;
	}

	buf[7] = RegsModified;

	// Compare memory
	//for (unsigned i = 0; i < 0x10000; i++) {
	//for (std::vector<uint8_t>::iterator it = mem.begin(); it != mem.end(); ++it) {
	for (unsigned i = 0; i < 0x10000; i++) {
		if ((*mem)[i] != (*m_OldMemory)[i]) {
			// TODO: Figure out how to write the address natively
			// although it might not be portable.
			// writing it in little endian for now
			buf[len] = (uint8_t)(i & 0xff);
			len++;
			buf[len] = (uint8_t)((i >> 8) & 0xff);
			len++;
			buf[len] = (*mem)[i];
			len++;
		}
		m_OldMemory->at(i) = (*mem)[i];
	}

	// len should now be at the appropriate length for this frame
	// so let's actually update the length byte and write it
	// doing it little-endian manually again.
	buf[4] = (len & 0xff);
	buf[5] = ((len >> 8) & 0xff);
	len += 2;
	for (unsigned i = 0; i < len; i++)
		*m_File << buf[i];

	m_File->flush();

	if (!m_File->good())
		throw;
}