; Helper functions for Silt.
;
; This file contains helper functions for the Silt system. Native code calls
; into here for things like stack manipulation.
;
; DL is used to pass 8-bit values, and DX is used to pass 16-bit values. This is
; for both parameters and return values.
;
; For reference, here is the VM register mapping, from System65Silt.hpp:
; * a  <-> AL
; * sp <-> AH
; * x  <-> BL
; * y  <-> BH
; * p  <-> CL
; * ip <-> DI

ifndef X64
.model flat, c
endif

.data
memory DWORD 0 ; Ptr to VM memory
stack  DWORD 0 ; Ptr to effective stack base

; push edi
; movzx edi,ah
; dec ah
; mov BYTE PTR [edi+<m_StackBase>],dh
; movzx edi,ah
; dec ah
; mov BYTE PTR [edi+<m_StackBase>],dl
; pop edi
_helper_push16_code db 057h,0Fh,0B6h,0FCh,0FEh,0CCh,88h,0B7h,00h,00h,00h,00h,0Fh,0B6h,0FCh,0FEh,0CCh,88h,97h,00h,00h,00h,00h,5Fh

.code
siltasm_init PROC
	push ebp
	mov ebp,esp
	push edx
	push ecx
	mov ecx,[ebp-4] ; memory base
	mov edx,[ebp-8] ; stack base
	mov memory,ecx  ; write
	mov stack,edx
	pop ecx
	pop edx
	pop ebp
	ret
siltasm_init ENDP

siltasm_push16 PROC
	push ecx
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov [ecx],dh
	dec ah
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov [ecx],dl
	dec ah
	pop ecx
	ret
siltasm_push16 ENDP

siltasm_puship PROC
	push ecx
	push edx
	mov dx,di
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov [ecx],dh
	dec ah
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov [ecx],dl
	dec ah
	pop edx
	pop ecx
	ret
siltasm_puship ENDP

siltasm_push8 PROC
	push ecx
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov [ecx],dl
	dec ah
	pop ecx
	ret
siltasm_push8 ENDP

siltasm_pop16 PROC
	push ecx
	inc ah
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov dl,[ecx]
	inc ah
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov dh,[ecx]
	pop ecx
	ret
siltasm_pop16 ENDP

siltasm_popip PROC
	push ecx
	push edx
	inc ah
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov dl,[ecx]
	inc ah
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov dh,[ecx]
	mov di,dx
	pop edx
	pop ecx
	ret
siltasm_popip ENDP

siltasm_pop8 PROC
	push ecx
	inc ah
	movzx ecx,ah
	lea ecx,[ecx+stack]
	mov dl,[ecx]
	pop ecx
	ret
siltasm_pop8 ENDP

END