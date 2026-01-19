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


; CLINK auto atomic_flag_test_and_set(bool* flag) -> bool;
; Args:
;   rdi -> flag
; Returns:
;   Value of flag before it was set to true.
global atomic_flag_test_and_set
atomic_flag_test_and_set:
    mov rax, 1
    lock xchg [rdi], rax
    ret

; CLINK void atomic_flag_clear(bool* flag);
; Args:
;   rdi -> flag
; Returns:
;   -
global atomic_flag_clear
atomic_flag_clear:
    lock and byte [rdi], 0
    ret

; CLINK auto atomic_flag_test(bool* flag) -> bool;
; Args:
;   rdi -> flag
; Returns:
;   The value of the flag
global atomic_flag_test
atomic_flag_test:
    lock and byte [rdi], 1
    jz .flag_not_set
    mov rax, 1
    ret
.flag_not_set:
    mov rax, 0
    ret