#include "System65Silt/System65Silt.hpp"

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------
System65Silt::System65Silt(unsigned int memsize, uint8_t stackbase)
{
	// Basic bounds checking
	if (memsize > 0x10000)
		memsize = 0x10000;

	if (memsize == 0)
		throw;

	m_Memory = new uint8_t[0x10000];
#if _DEBUG
	assert(m_Memory != NULL);
#endif // _DEBUG
	memset(m_Memory, 0, sizeof(uint8_t)*0xffff);
	memorysize = memsize;
	memset(&m_Register, 0, sizeof(m_Register)); // maybe superfluous

	m_StackBase = stackbase << 8;
	m_EffectiveStackBase = m_Memory+m_StackBase;
	siltasm_init(m_Memory,m_EffectiveStackBase);
}

System65Silt::~System65Silt()
{
	// Free the cache
	for (CacheMap::iterator iter = m_Cache.begin(); iter != m_Cache.end(); ++iter)
		::VirtualFree(iter->second.ptr, iter->second.size, MEM_DECOMMIT | MEM_RELEASE);

	delete []m_Memory;
}

void System65Silt::Tick(unsigned int cycleLimit)
{
	const NativeCode *code;

	// check to see if the compiled code already exists in our cache
	CacheMap::const_iterator iter = m_Cache.find(m_Register.pc);
	if (iter != m_Cache.end())
		code = &iter->second;
	else
		code = Compile(m_Register.pc);

	// run it
	Execute(code);
}

uint8_t System65Silt::SetStackBasePage(uint8_t newbase)
{
	uint8_t oldbase = m_StackBase >> 8;
	m_StackBase = newbase << 8;
	return oldbase;
}

//------------------------------------------------------------------------------
// Protected
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private
//------------------------------------------------------------------------------

const System65Silt::NativeCode *System65Silt::Compile(uint16_t iptr)
{
	const uint8_t *in = &(*this)[iptr];
	NativeCode code;

	// First-pass compile to determine how much buffer we need
	// The +1 is for the native return
	code.size = CompileBlock(in, NULL) + 1;

#ifdef WIN32
	// Windows only: allow exec in this memory block
	code.ptr = static_cast<uint8_t *>(::VirtualAlloc(NULL, code.size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
#endif // WIN32

	// Second pass for actual compile
	CompileBlock(in, code.ptr);

	// Then add the x86 return
	code.ptr[code.size - 1] = 0xc3;

	// Finally store it in the cache
	m_Cache[iptr] = code;
	return &m_Cache[iptr];
}

int System65Silt::CompileBlock(const uint8_t *in, uint8_t *out)
{
	int cyclecount;

	// in: ptr to the instruction byte to translate
	// out: ptr to the next free space in the buffer for emitting native assembly (or NULL if not emitting anything)
	// c: number of target (6502) cycles for this instruction
	// stop: whether or not compilation should stop, should generally happen at a branch
	// return value: size of the native code emitted
	typedef int (System65Silt::*CompileFunc)(const uint8_t *&in, uint8_t *&out, int &count, bool &stop);

	static const CompileFunc opcodeTable[0x100] = {
		// 00h                     01h                     02h                     03h                     04h                     05h                     06h                     07h                     08h                     09h                     0Ah                     0Bh                     0Ch                     0Dh                     0Eh                     0Fh
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // 00h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // 10h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // 20h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // 30h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // 40h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // 50h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // 60h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // 70h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // 80h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,&System65Silt::i_staabx,nullptr                ,nullptr                , // 90h
		   nullptr                ,&System65Silt::i_ldainx,&System65Silt::i_ldximm,nullptr                ,nullptr                ,&System65Silt::i_ldazpg,nullptr                ,nullptr                ,nullptr                ,&System65Silt::i_ldaimm,nullptr                ,nullptr                ,nullptr                ,&System65Silt::i_ldaabs,nullptr                ,nullptr                , // A0h
		   nullptr                ,&System65Silt::i_ldainy,nullptr                ,nullptr                ,nullptr                ,&System65Silt::i_ldazpx,nullptr                ,nullptr                ,nullptr                ,&System65Silt::i_ldaaby,nullptr                ,nullptr                ,nullptr                ,&System65Silt::i_ldaabx,nullptr                ,nullptr                , // B0h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // C0h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // D0h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,&System65Silt::i_nop   ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                , // E0h
		   nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                ,nullptr                  // F0h
	};

	bool stop = false;
	int size = 0;

	while (!stop)
		size += (*this.*opcodeTable[*in])(in, out, cyclecount, stop);

	return size;
}

void System65Silt::Execute(const NativeCode *code)
{
	const uint8_t *native = code->ptr;
	Registers *state = &m_Register;

	__asm {
		// save all of the registers
		pushad

		// Load the VM state
		mov edx, state
		mov di, [edx]Registers.pc
		mov al, [edx]Registers.a
		mov ah, [edx]Registers.s
		mov bl, [edx]Registers.x
		mov bh, [edx]Registers.y
		mov cl, [edx]Registers.p

		// Begin execution
		call native

		// Save VM state
		mov edx, state
		mov [edx]Registers.pc, di
		mov [edx]Registers.a, al
		mov [edx]Registers.s, ah
		mov [edx]Registers.x, bl
		mov [edx]Registers.y, bh
		mov [edx]Registers.p, cl

		// pop all registers back
		popad
	}
}