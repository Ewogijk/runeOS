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

; The macros will start the arrangement of the stack to represent the interrupt frame.
; The CPU will push ss, rsp, rflags, cs and rip all the time and for some exceptions an
; optional error code. In cases where no error code is pushed a dummy code of zero will be pushed instead.
; Upon entry of the "isr_common" label the stack will be in the following state:
; - It has enough space for the whole interrupt frame, preserved registers and the CPU pushed registers
; - The interrupt vector and error code will be saved at the correct position
;
; The state of the stack after jumping to "isr_common", "??" denotes that the stack content at this position
; is undefined:
; ---------------------
; |       Stack       |
; ---------------------
; |      SS           | <-- RSP + 272   - Interrupt handler stack/x86InterruptContext start - pushed by CPU
; |      RSP          | <-- RSP + 264
; |      RFLAGS       | <-- RSP + 256
; |      CS           | <-- RSP + 248
; |      RIP          | <-- RSP + 240
; |      Error Code   | <-- RSP + 232   - Interrupt handler stack end - iretq expects only these values on the stack
; |      Vector       | <-- RSP + 224
; |      ??           | <-- RSP + 216
; |      ??           | <-- RSP + 208
; |      ??           | <-- RSP + 200
; |      ??           | <-- RSP + 192
; |      ??           | <-- RSP + 184
; |      ??           | <-- RSP + 176
; |      ??           | <-- RSP + 168
; |      ??           | <-- RSP + 160
; |      ??           | <-- RSP + 152
; |      ??           | <-- RSP + 144
; |      ??           | <-- RSP + 136
; |      ??           | <-- RSP + 128
; |      ??           | <-- RSP + 120
; |      ??           | <-- RSP + 112
; |      ??           | <-- RSP + 104
; |      ??           | <-- RSP + 96
; |      ??           | <-- RSP + 88
; |      ??           | <-- RSP + 80
; |      ??           | <-- RSP + 72
; |      ??           | <-- RSP + 64
; |      ??           | <-- RSP + 56
; |      ??           | <-- RSP + 48
; |      ??           | <-- RSP + 40
; |      ??           | <-- RSP + 32
; |      ??           | <-- RSP + 24
; |      ??           | <-- RSP + 16
; |      ??           | <-- RSP + 8
; |      RAX          | <-- RSP         - x86InterruptContext start
; ---------------------
%macro ISR_NOERRORCODE 1
global ISR%1:
ISR%1:
    sub rsp, 240

    mov [rsp], rax          ; Push RAX so we can use it as temp register
    mov rax, 0
    mov [rsp + 232], rax    ; Push the dummy error code of zero
    mov rax, %1
    mov [rsp + 224], rax    ; Push the interrupt vector

    jmp isr_common
%endmacro

%macro ISR_ERRORCODE 1
global ISR%1:
ISR%1:
    sub rsp, 232

    mov [rsp], rax          ; Save RAX so we can use it as temp register
    ; The CPU has already pushed the error code
    mov rax, %1
    mov [rsp + 224], rax    ; Push the interrupt vector

    jmp isr_common
%endmacro


; When an interrupt happens we have to check wether we come from user mode or kernel mode
; If we come from user mode we want to swap the pointers to the thread stack in the
; GSBase and KernelGSBase MSR's so that GS points to the kernel stack rather than the user stack
; If we come from kernel mode we do not want to swap them, because GS points already to the kernel stack
%macro SWAPGS_IF_NECESSARY 0
    mov rax, [rsp + 248]    ; called in isr_common -> rax is free and stack is setup
    and rax, 0x03           ; Get cpl of cs
    cmp rax, 0x03           ; check if cpl == 3
    jne %%no_swap           ; if cpl == 3 -> interrupt triggered in user space -> call swapgs
    swapgs
%%no_swap:
%endmacro


%include "CPU/Interrupt/ISR_Stubs.inc"


isr_common:
    ; While we only pass the interrupt vector and error code to c++
    ; we still need to save the whole CPU state, because for interrupts
    ; the normal System V ABI does not apply as the running code has no
    ; clue it is interrupted
    ; Missing out on some registers will lead to undefined behavior and
    ; we relly do not want to have that here
    ; ---------------------
    ; |       Stack       |
    ; ---------------------
    ; |      SS           | <-- RSP + 272   - Interrupt handler stack/x86InterruptContext start - pushed by CPU
    ; |      RSP          | <-- RSP + 264
    ; |      RFLAGS       | <-- RSP + 256
    ; |      CS           | <-- RSP + 248
    ; |      RIP          | <-- RSP + 240   - iretq expects only values up to here on the stack
    ; |      Error Code   | <-- RSP + 232   - Interrupt handler stack end
    ; |      Vector       | <-- RSP + 224
    ; |      GS           | <-- RSP + 216
    ; |      FS           | <-- RSP + 208
    ; |      ES           | <-- RSP + 200
    ; |      SS           | <-- RSP + 192
    ; |      DS           | <-- RSP + 184
    ; |      CS           | <-- RSP + 176
    ; |      CR4          | <-- RSP + 168
    ; |      CR3          | <-- RSP + 160
    ; |      CR2          | <-- RSP + 152
    ; |      CR0          | <-- RSP + 144
    ; |      RFlags       | <-- RSP + 136
    ; |      RIP          | <-- RSP + 128
    ; |      RBP          | <-- RSP + 120
    ; |      RSP          | <-- RSP + 112
    ; |      R15          | <-- RSP + 104
    ; |      R14          | <-- RSP + 96
    ; |      R13          | <-- RSP + 88
    ; |      R12          | <-- RSP + 80
    ; |      R11          | <-- RSP + 72
    ; |      R10          | <-- RSP + 64
    ; |      R9           | <-- RSP + 56
    ; |      R8           | <-- RSP + 48
    ; |      RDI          | <-- RSP + 40
    ; |      RSI          | <-- RSP + 32
    ; |      RDX          | <-- RSP + 24
    ; |      RCX          | <-- RSP + 16
    ; |      RBX          | <-- RSP + 8
    ; |      RAX          | <-- RSP         - x86InterruptContext start
    ; ---------------------

    ; General purpose registers
    ; RAX already pushed
    mov [rsp + 8], rbx
    mov [rsp + 16], rcx
    mov [rsp + 24], rdx
    mov [rsp + 32], rsi
    mov [rsp + 40], rdi
    mov [rsp + 48], r8
    mov [rsp + 56], r9
    mov [rsp + 64], r10
    mov [rsp + 72], r11
    mov [rsp + 80], r12
    mov [rsp + 88], r13
    mov [rsp + 96], r14
    mov [rsp + 104], r15

    ; Special registers
    mov [rsp + 112], rsp
    mov [rsp + 120], rbp

    ; rip is not a addressable register therefore we need to use a trick
    ; we load the address of the "give_rip" label to which rip actually points at the moment into rax
    ; and then save rax
    lea rax, [rel give_rip]
give_rip:
    mov [rsp + 128], rax

    ; rflags cannot be addressed directly
    ; however we can push it onto the stack then pop it into rax
    pushfq
    pop rax
    mov [rsp + 136], rax

    ; it is not allowed to load control registers directly to a memory location
    ; thats why it is first loaded to rax and then rax to memory
    mov rax, cr0
    mov [rsp + 144], rax
    mov rax, cr2
    mov [rsp + 152], rax
    mov rax, cr3
    mov [rsp + 160], rax
    mov rax, cr4
    mov [rsp + 168], rax

    ; Segment selectors
    ; Moving the segment selectors directly to memory loads literaly random values
    ; First loading to rax then to memory is fine tho
    mov rax, cs
    mov [rsp + 176], rax
    mov rax, ds
    mov [rsp + 184], rax
    mov rax, ss
    mov [rsp + 192], rax
    mov rax, es
    mov [rsp + 200], rax
    mov rax, fs
    mov [rsp + 208], rax
    mov rax, gs
    mov [rsp + 216], rax

    SWAPGS_IF_NECESSARY     ; Swap to kernel stack if we come from CPL=3

    mov rdi, rsp
    extern interrupt_dispatch
    call interrupt_dispatch

    SWAPGS_IF_NECESSARY     ; Swap to user stack if we return to CPL=3

    ; Restore registers and prepare stack for iretq
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    add rsp, 8      ; Pop rsp -> We dont want to restore that one
    pop rbp
    add rsp, 112    ; Pop rip, rflags, cr0, cr2, cr3, cr4, cs, ds, ss, es, fs, gs, vector and error code

    iretq
