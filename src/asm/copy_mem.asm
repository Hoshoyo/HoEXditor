.data

.code

; code to check divisibility by 16
; mov rax, 0x0f
; and rax, [argument]
; jz divisible
; ; not divisible here

copy_mem proc

; rdx = ptr to src
; rcx = ptr to dest
; r8 = size

push rbp
mov	rbp, rsp
push r10

xor rax, rax		; count = 0
					; curr_size = r8
					
cmp r8, 0			; if(size == 0) return 0;
jne start
pop r10
leave
ret

start:
cmp r8, 128				; if(curr_size < 128)
jge unaligned_128b_mov	;

try_32:
cmp r8, 32								; if(curr_size >= 32)
jl try_16								;

; copy 32 bytes
movups  xmm0, [rdx]						; copy 32 bytes
movups  xmm1, 16[rdx]					;
movups  xmmword ptr[rcx], xmm0			;
movups  xmmword ptr[rcx + 16], xmm1		;
sub r8, 32								; curr_size -= 32
add rcx, 32								; dest = (char*)dest + 32
add rdx, 32								; src = (char*)src + 32
;add rax, 32								; count += 32
jmp try_32

try_16:
cmp r8, 16								; if(curr_size >= 16)
jl try_8								;

; copy 16 bytes
movups  xmm0, [rdx]						; copy 16 bytes
movups  xmmword ptr[rcx], xmm0			;
sub r8, 16								; curr_size -= 16
add rcx, 16								; dest = (char*)dest + 16
add rdx, 16								; src = (char*)src + 16
;add rax, 16								; count += 16

try_8:
cmp r8, 8								; if(curr_size >= 8)
jl try_4								;

; copy 8 bytes
mov r10, qword ptr[rdx]					; copy 8 bytes
mov qword ptr[rcx], r10					;
sub r8, 8								; curr_size -= 8
add rcx, 8								; dest = (char*)dest + 8
add rdx, 8								; src = (char*)src + 8
;add rax, 8								; count += 8

try_4:
cmp r8, 4								; if(curr_size >= 4)
jl check_end							;

; copy 4 bytes
mov r10d, dword ptr[rdx]				; copy 4 bytes
mov dword ptr[rcx], r10d				;
sub r8, 4								; curr_size -= 4
add rcx, 4								; dest = (char*)dest + 4
add rdx, 4								; src = (char*)src + 4
;add rax, 4								; count += 4

check_end:
test r8, r8								; if(curr_size == 0)
jne skip_end
pop r10
leave
ret										; return 0
skip_end:
; copy 1 byte
mov r10b, byte ptr[rdx]					; copy 1 byte
mov byte ptr[rcx], r10b					;
dec r8									; curr_size -= 1
inc rcx									; dest = (char*)dest + 1
inc rdx									; src = (char*)src + 1
;inc rax									; count += 1
jmp check_end

unaligned_128b_mov:
; brute copy of 128 bytes not aligned
movups  xmm0, [rdx]			; 16
movups  xmm1, 16[rdx]		; 32
movups  xmm2, 32[rdx]		; 48
movups  xmm3, 48[rdx]		; 64

movups  xmmword ptr[rcx], xmm0
movups  xmmword ptr[rcx + 16], xmm1
movups  xmmword ptr[rcx + 32], xmm2
movups  xmmword ptr[rcx + 48], xmm3

movups  xmm0, 64[rdx]		; 80
movups  xmm1, 80[rdx]		; 96
movups  xmm2, 96[rdx]		; 112
movups  xmm3, 112[rdx]		; 128

movups  xmmword ptr[rcx + 64], xmm0
movups  xmmword ptr[rcx + 80], xmm1
movups  xmmword ptr[rcx + 96], xmm2
movups  xmmword ptr[rcx + 112], xmm3

sub r8, 128								; curr_size -= 128
add rcx, 128							; dest = (char*)dest + 128
add rdx, 128							; src = (char*)src + 128
;add rax, 128							; count += 128

jmp start

copy_mem endp


copy_mem_aligned proc

; rdx = ptr to src
; rcx = ptr to dest
; r8 = size

push rbp
mov	rbp, rsp
push r10

xor rax, rax		; count = 0
					; curr_size = r8
					
cmp r8, 0			; if(size == 0) return 0;
jne start
pop r10
leave
ret

start:
cmp r8, 128				; if(curr_size < 128)
jge unaligned_128b_mov	;

try_32:
cmp r8, 32								; if(curr_size >= 32)
jl try_16								;

; copy 32 bytes
movups  xmm0, [rdx]						; copy 32 bytes
movups  xmm1, 16[rdx]					;
movups  xmmword ptr[rcx], xmm0			;
movups  xmmword ptr[rcx + 16], xmm1		;
sub r8, 32								; curr_size -= 32
add rcx, 32								; dest = (char*)dest + 32
add rdx, 32								; src = (char*)src + 32
add rax, 32								; count += 32
jmp try_32

try_16:
cmp r8, 16								; if(curr_size >= 16)
jl try_8								;

; copy 16 bytes
movups  xmm0, [rdx]						; copy 16 bytes
movups  xmmword ptr[rcx], xmm0			;
sub r8, 16								; curr_size -= 16
add rcx, 16								; dest = (char*)dest + 16
add rdx, 16								; src = (char*)src + 16
add rax, 16								; count += 16

try_8:
cmp r8, 8								; if(curr_size >= 8)
jl try_4								;

; copy 8 bytes
mov r10, qword ptr[rdx]					; copy 8 bytes
mov qword ptr[rcx], r10					;
sub r8, 8								; curr_size -= 8
add rcx, 8								; dest = (char*)dest + 8
add rdx, 8								; src = (char*)src + 8
add rax, 8								; count += 8

try_4:
cmp r8, 4								; if(curr_size >= 4)
jl check_end							;

; copy 4 bytes
mov r10d, dword ptr[rdx]				; copy 4 bytes
mov dword ptr[rcx], r10d				;
sub r8, 4								; curr_size -= 4
add rcx, 4								; dest = (char*)dest + 4
add rdx, 4								; src = (char*)src + 4
add rax, 4								; count += 4

check_end:
test r8, r8								; if(curr_size == 0)
jne skip_end
pop r10
leave
ret										; return 0
skip_end:
; copy 1 byte
mov r10b, byte ptr[rdx]					; copy 1 byte
mov byte ptr[rcx], r10b					;
dec r8									; curr_size -= 1
inc rcx									; dest = (char*)dest + 1
inc rdx									; src = (char*)src + 1
inc rax									; count += 1
jmp check_end

unaligned_128b_mov:
; brute copy of 128 bytes not aligned
movaps  xmm0, [rdx]			; 16
movaps  xmm1, 16[rdx]		; 32
movaps  xmm2, 32[rdx]		; 48
movaps  xmm3, 48[rdx]		; 64

movaps  xmmword ptr[rcx], xmm0
movaps  xmmword ptr[rcx + 16], xmm1
movaps  xmmword ptr[rcx + 32], xmm2
movaps  xmmword ptr[rcx + 48], xmm3

movaps  xmm0, 64[rdx]		; 80
movaps  xmm1, 80[rdx]		; 96
movaps  xmm2, 96[rdx]		; 112
movaps  xmm3, 112[rdx]		; 128

movaps  xmmword ptr[rcx + 64], xmm0
movaps  xmmword ptr[rcx + 80], xmm1
movaps  xmmword ptr[rcx + 96], xmm2
movaps  xmmword ptr[rcx + 112], xmm3

sub r8, 128								; curr_size -= 128
add rcx, 128							; dest = (char*)dest + 128
add rdx, 128							; src = (char*)src + 128
;add rax, 128							; count += 128

jmp start

copy_mem_aligned endp

end