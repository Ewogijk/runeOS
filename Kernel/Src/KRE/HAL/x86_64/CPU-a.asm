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

; CLINK Register cpu_get_stack_pointer();
; Args:
;   -
; Returns:
;   Current value of the stack pointer.
global cpu_get_stack_pointer
cpu_get_stack_pointer:
    mov rax, rsp
    add rax, 8      ; Account for the return address being popped on ret
    ret


; CLINK void cpu_halt();
; Args:
;   -
; Returns:
;   -
global cpu_halt:
cpu_halt:
    hlt

; CLINK void pause();
; Args:
;   -
; Returns:
;   -
global cpu_pause
cpu_pause:
    pause


; CLINK Register get_page_fault_address();
; Args:
;   rdi -> -
;   rsi -> -
;   rdx -> -
;   rcx -> -
; Returns:
;   The content of the cr2 register.
global cpu_get_page_fault_address
cpu_get_page_fault_address:
    mov rax, cr2
    ret