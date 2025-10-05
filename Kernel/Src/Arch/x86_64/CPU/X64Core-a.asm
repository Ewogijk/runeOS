;
; Copyright 2025 Ewogijk
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                          CPU State
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; CLINK void read_state(State* state);
; Args:
;   rdi -> state
; Returns:
;   -
global read_state
read_state:
    ; We will fill in the fields for the CPU::State struct
    ; General purpose registers
    mov [rdi], rax
    mov [rdi + 8], rbx
    mov [rdi + 16], rcx
    mov [rdi + 24], rdx
    mov [rdi + 32], rsi
    mov [rdi + 40], rdi
    mov [rdi + 48], r8
    mov [rdi + 56], r9
    mov [rdi + 64], r10
    mov [rdi + 72], r11
    mov [rdi + 80], r12
    mov [rdi + 88], r13
    mov [rdi + 96], r14
    mov [rdi + 104], r15

    ; Special registers
    mov [rdi + 112], rsp
    mov [rdi + 120], rbp

    ; rip is not a addressable register therefore we need to use a trick
    ; we load the address of the "give_rip" label to which rip actually points at the moment into rax
    ; and then save rax
    lea rax, [rel give_rip]
give_rip:
    mov [rdi + 128], rax

    ; rflags cannot be addressed directly
    ; however we can push it onto the stack then pop it into rax
    pushfq
    pop rax
    mov [rdi + 136], rax

    ; it is not allowed to load control registers directly to a memory location
    ; thats why it is first loaded to rax and then rax to memory
    mov rax, cr0
    mov [rdi + 144], rax
    mov rax, cr2
    mov [rdi + 152], rax
    mov rax, cr3
    mov [rdi + 160], rax
    mov rax, cr4
    mov [rdi + 168], rax

    ; Segment selectors
    ; Moving the segment selectors directly to memory loads literaly random values
    ; First loading to rax then to memory is fine tho
    mov rax, cs
    mov [rdi + 176], rax
    mov rax, ds
    mov [rdi + 184], rax
    mov rax, ss
    mov [rdi + 192], rax
    mov rax, es
    mov [rdi + 200], rax
    mov rax, fs
    mov [rdi + 208], rax
    mov rax, gs
    mov [rdi + 216], rax


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                          CPUID Features
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;; CLINK bool cpuid_supported();
;; Args:
;;   -
;;
;global cpuid_supported
;cpuid_supported:
;    push rbp
;    mov rbp, rsp
;
;    pushfq                  ; Save original rflags
;
;    pushfq                  ; Store rflags for modification
;    mov rax, [rsp]          ; Invert ID bit
;    xor rax, 0x00200000
;    mov [rsp], rax
;    popfq                   ; Load modified rflags
;
;    pushfq                  ; Save modified rflags
;    pop rax
;    xor rax, [rsp]          ; rax contains all modified bits
;
;    popfq                   ; Restore original rflags
;
;    and rax, 0x00200000     ; Check if ID bit (position 21) is modified
;    shr rax, 21
;
;    leave
;    ret
;
;
;; CLINK void make_cpuid_request(U64 request, CPUIDResponse* resp);
;; Args:
;;   rdi -> request
;;   rsi -> resp
;; Returns:
;;   -
;global make_cpuid_request
;make_cpuid_request:
;    ; make new call frame
;    push rbp            ; save old call frame
;    mov rbp, rsp        ; initialize new call frame
;
;    push rbx
;
;    mov rax, rdi
;    cpuid
;
;    mov [rsi], rax
;    mov [rsi + 8], rbx
;    mov [rsi + 16], rcx
;    mov [rsi + 24], rdx
;
;    pop rbx
;
;    ; restore old call frame
;    mov rsp, rbp
;    pop rbp
;    ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                          Model specific registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; CLINK void write_msr(Register msrID, Register value);
; Args:
;   rdi -> msrID
;   rsi -> value
; Returns:
;   -
global write_msr
write_msr:
    mov rcx, rdi
    mov rdx, rsi
    shr rdx, 32         ; RDX contains upper 32 bits of the value in it's lower 32 bit
    mov rax, rsi
    wrmsr
    ret


; CLINK Register read_msr(Register msrID);
; Args:
;   rdi -> msrID
; Returns:
;   rax -> msr value
global read_msr
read_msr:
    mov rcx, rdi
    rdmsr
    shl rdx, 32
    or rax, rdx         ; RAX has lower 32-bit and RDX higher 32-bit
    ret


; CLINK Register read_gs();
; Args:
;   -
; Returns:
;   rax -> Value of gs
global read_gs
read_gs:
    mov rax, [gs:0]
    ret


; CLINK void swapgs();
; Args:
;   -
; Returns:
;   -
global swapgs
swapgs:
    swapgs
    ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                          Threading functions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; CLINK void exec_kernel_mode(Register start_info, Register func_ptr, Register thread_exit);
; Args:
;   rdi -> start_info
;   rsi -> func_ptr
;   rdx -> thread_exit
; Returns:
;   -
global exec_kernel_mode
exec_kernel_mode:
    ; make new call frame
    push rbp            ; save old call frame
    mov rbp, rsp        ; initialize new call frame

    push rdx            ; Save threadExit so we can call it later
    call rsi            ; Call thread main

    mov rdi, rax        ; Pass the exit code to threadExit
    pop rdx             ; Restore threadExit
    call rdx            ; Call threadExit


    mov rsp, rbp
    pop rbp
    ; Return to ThreadEnter
    ; this may or may no be called depending on how fast the thread is terminated
    ; so put it here just in case or everything will be fucked
    ret


; CLINK void exec_user_mode(Register start_info, Register func_ptr);
; Args:
;   rdi -> start_info
;   rsi -> func_ptr
; Returns:
;   -
global exec_user_mode
exec_user_mode:
    swapgs
    mov rsp, [gs:0] ; Load user stack

    mov rcx, rsi    ; Load funcPtr into rcx
    mov r11, 0x202  ; 0x200 -> Enable Interrupts, 0x002 -> Reserved, always 1
    o64 sysret      ; Loads rcx -> rip and r11 -> rflags


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                          Other assembly stuff
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; CLINK void enable_sse();
; Args:
;   -
; Returns:
;   -
global enable_sse:
enable_sse:
    ; make new call frame
    push rbp            ; save old call frame
    mov rbp, rsp        ; initialize new call frame

    mov rax, 0x1
    cpuid
    test rdx, 1<<25
    jz .noSSE

    mov rax, cr0
    and ax, 0xFFFB		;clear coprocessor emulation CR0.EM
    or ax, 0x2			;set coprocessor monitoring  CR0.MP
    mov cr0, rax
    mov rax, cr4
    or ax, 3 << 9		;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
    mov cr4, rax
.noSSE:
    ; restore old call frame
    mov rsp, rbp
    pop rbp
    ret


; CLINK Register read_cs();
; Args:
;   -
; Returns:
; rax -> Value of the CS register
global read_cs
read_cs:
    mov rax, cs


; CLINK void context_switch_ass(
;             LibK::VirtualAddr* c_stack,
;             LibK::PhysicalAddr c_vas,
;             LibK::VirtualAddr n_stack,
;             LibK::PhysicalAddr n_vas
;     );
; Args:
;   rdi -> c_stack
;   rsi -> c_vas
;   rdx -> n_stack
;   rcx -> n_vas
; Returns:
;   -
global context_switch_ass
context_switch_ass:
    ; !!! Important Note !!!
    ; setup_trampoline_kernel_stack() in Stack.cpp initializes the stack for new threads and must replicate the
    ; stack layout defined here so that the very first context switch to a thread does not corrupt the stack.
    ; Therefore whenever something here changes setup_trampoline_kernel_stack() must be adjusted accordingly (but never
    ; the other way around)


    ; Registers that need no saving:
    ; ES,CS,SS,DS,FS,GS                 -> These are constants
    ; RIP                               -> Only the return address is interesting, which is already on the stack
    ; RSP, CR3                          -> Saved in the thread struct
    ; RAX,RDI,RSI,RDX,RCX,R8,R9,R10,R11 -> Scratch registers, the caller saves them if used

    ; Preserved general purpose registers
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    ; SSE registers used for floating point numbers
    ; System V ABI say those are not preserved therefore we should not need to save them but bugssss therefore we need
    ; These registers are 128 bits wide but there is no 128 bit push instruction, so we have to emulate push
    sub rsp, 16
    movdqu [rsp], xmm0
    sub rsp, 16
    movdqu [rsp], xmm1
    sub rsp, 16
    movdqu [rsp], xmm2
    sub rsp, 16
    movdqu [rsp], xmm3
    sub rsp, 16
    movdqu [rsp], xmm4
    sub rsp, 16
    movdqu [rsp], xmm5
    sub rsp, 16
    movdqu [rsp], xmm6
    sub rsp, 16
    movdqu [rsp], xmm7
    sub rsp, 16
    movdqu [rsp], xmm8
    sub rsp, 16
    movdqu [rsp], xmm9
    sub rsp, 16
    movdqu [rsp], xmm10
    sub rsp, 16
    movdqu [rsp], xmm11
    sub rsp, 16
    movdqu [rsp], xmm12
    sub rsp, 16
    movdqu [rsp], xmm13
    sub rsp, 16
    movdqu [rsp], xmm14
    sub rsp, 16
    movdqu [rsp], xmm15

    mov [rdi], rsp      ; Update current threads stack
    mov rsp, rdx        ; Swap current threads stack against next threads stack

    cmp rsi, rcx        ; Check if VAS needs to be swapped
    je .VASChangeDone
    mov cr3, rcx        ; Load the new VAS

.VASChangeDone:
    ; Restore the preserved registers of the new thread
    ; NOTE: If this changes -> UPDATE the stack layout as appropriate

    ; Pop emulation for 128 bit SSE values
    movdqu xmm15, [rsp]
    add rsp, 16
    movdqu xmm14, [rsp]
    add rsp, 16
    movdqu xmm13, [rsp]
    add rsp, 16
    movdqu xmm12, [rsp]
    add rsp, 16
    movdqu xmm11, [rsp]
    add rsp, 16
    movdqu xmm10, [rsp]
    add rsp, 16
    movdqu xmm9, [rsp]
    add rsp, 16
    movdqu xmm8, [rsp]
    add rsp, 16
    movdqu xmm7, [rsp]
    add rsp, 16
    movdqu xmm6, [rsp]
    add rsp, 16
    movdqu xmm5, [rsp]
    add rsp, 16
    movdqu xmm4, [rsp]
    add rsp, 16
    movdqu xmm3, [rsp]
    add rsp, 16
    movdqu xmm2, [rsp]
    add rsp, 16
    movdqu xmm1, [rsp]
    add rsp, 16
    movdqu xmm0, [rsp]
    add rsp, 16

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret