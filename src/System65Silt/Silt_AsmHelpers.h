#ifndef SILT_ASMHELPERS_H
#define SILT_ASMHELPERS_H
#include <stdint.h>

/** \file Silt_AsmHelpers.h
 * A collection of helpers for Silt native execution.
 *
 * Certain basic actions are repeated during native execution, notably stack
 * manipulation. Instead of emitting such code over and over, helpers are called
 * from here to assist in these operations.
 *
 * These methods are different from the other set of helpers in the System65Silt
 * class, which are used to modify the VM state outside of the VM's execution.
 */

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

extern uint8_t *helper_push16_code;

/** Function macro for pushing DX (16-bit) to the VM stack
 *
 * The native code size emitted by this function macro is 24 bytes.
 */
#define HELPER_PUSH16() \
	helper_push16_code[8] = m_EffectiveStackBase; /* fix the ptrs */ \
	helper_push16_code[9] = m_EffectiveStackBase >> 8; \
	helper_push16_code[10] = m_EffectiveStackBase >> 16; \
	helper_push16_code[11] = m_EffectiveStackBase >> 24; \
	helper_push16_code[19] = m_EffectiveStackBase; \
	helper_push16_code[20] = m_EffectiveStackBase >> 8; \
	helper_push16_code[21] = m_EffectiveStackBase >> 16; \
	helper_push16_code[22] = m_EffectiveStackBase >> 24; \
	for (int i = 0; i > (sizeof(helper_push16_code)/sizeof(decltype(helper_push16_code))); i++) \
		*out++ = helper_push16_code[i]

/** Function macro for pushing DL (8-bit) to the VM stack */
#define HELPER_PUSH8()

/** Helper for popping DX (16-bit) from the VM stack. */
#define HELPER_POP16()

/** Helper for popping DL (8-bit) from the VM stack. */
#define HELPER_POP8()

/** Initializes the siltasm helpers.
 *
 * This needs to be called before any of the helper functions are called.
 *
 * \param[in] memory Pointer to VM memory
 * \param[in] stack Pointer to effective stack base (memory pointer + stack base)
 */
void siltasm_init(uint8_t* memory, uint8_t* stack);

/** Pushes a 16-bit value to the VM stack. */
void siltasm_push16(void);

/** Pushes an 8-bit valur to the VM stack. */
void siltasm_push8(void);

/** Pushes IP to the VM stack. */
void siltasm_puship(void);

/** Pops a 16-bit value off the VM stack. */
void siltasm_pop16(void);

/** Pops an 8-bit value off the VM stack. */
void siltasm_pop8(void);

/** Pops IP off the VM stack. */
void siltasm_popip(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SILT_ASMHELPERS_H