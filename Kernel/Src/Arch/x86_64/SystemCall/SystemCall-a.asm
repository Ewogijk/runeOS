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

; CLINK void system_call_accept();
; Args:
;   -
; Returns:
;   -
global system_call_accept
system_call_accept:
    mov [gs:0], rsp ; Update the current value of the user stack
    swapgs
    mov rsp, [gs:0] ; Load kernel stack

    push rcx    ; Save return user mode rip
    push r11    ; Save user mode rflags

    push r14    ; Save user mode values
    push r15    ; These are used as tmp registers for syscall args

    ; System Call ABI:
    ; | Handle | Arg1 | Arg2 | Arg3 | Arg4 | Arg5 | Arg6 |
    ; | rax    | rdi  | rsi  | rdx  | r8   | r9   | r10  |
    ; Need to place them in System V ABI registers for parameter passing
    ; | Handle | Arg1 | Arg2 | Arg3 | Arg4 | Arg5 | Arg6  |
    ; | rdi    | rsi  | rdx  | rcx  | r8   | r9   | stack |
    mov r15, rdi    ; r15=Arg1
    mov rdi, rax    ; rdi=Handle

    mov r14, rsi    ; r14=Arg2
    mov rsi, r15    ; rsi=Arg1

    mov r15, rdx    ; r15=Arg3
    mov rdx, r14    ; rdx=Arg2

    mov rcx, r15    ; rcx=Arg3
    ; Already good: r8=Arg4 and r9=Arg5
    push r10        ; Need to pass Arg6 via stack


    ; SysCallArgs is already in rdi, so there is nothing to do here
    extern system_call_dispatch
    call system_call_dispatch

    pop r10     ; Remove Arg6

    pop r15     ; Restore user mode registers
    pop r14

    pop r11     ; Restore user mode rflags
    pop rcx     ; Restore return user mode rip

    swapgs
    mov rsp, [gs:0] ; Load user stack
    o64 sysret