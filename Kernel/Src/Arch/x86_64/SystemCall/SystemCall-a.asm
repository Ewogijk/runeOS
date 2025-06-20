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

    ; SysCallArgs is already in rdi, so there is nothing to do here
    extern system_call_dispatch
    call system_call_dispatch


    pop r11     ; Restore user mode rflags
    pop rcx     ; Restore return user mode rip

    swapgs
    mov rsp, [gs:0] ; Load user stack
    o64 sysret