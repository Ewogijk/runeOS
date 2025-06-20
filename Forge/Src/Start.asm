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

global _start:
_start:
    ; Initialize global vars of liballoc because global constructors are rip.
    ; An OS specific toolchain is required for global constructors which in return
    ; needs a libc implementation -> HUMONGOUS WORK -> that's why this workaround atm
    extern liballoc_init
    call liballoc_init

    extern main     ; Run the app
    call main
    push rax        ; Save the exit code on the stack

    mov rdi, rax
    extern app_exit
    call app_exit