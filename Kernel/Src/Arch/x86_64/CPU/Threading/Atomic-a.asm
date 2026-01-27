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


; CLINK auto atomic_load(volatile int* value) -> int;
; Args:
;   rdi -> value
; Returns:
;   The value loaded at the memory address of value.
global atomic_load
atomic_load_relaxed:
    ; mov is atomic as long as the address is 4-byte memory aligned
    ; Reasoning is if it is memory aligned then it fits in a single cache line
    ; Misaligned memory might not fit in a single cacheline, thus requires two reads
    ; -> atomicity is lost
    ; This implementation assumes only memory aligned addresses are given,
    mov rax, [rdi]
    ret

; CLINK auto atomic_load_acquire(volatile int* value) -> int;
; Args:
;   rdi -> value
; Returns:
;   The value loaded at the memory address of value.
global atomic_load_acquire
atomic_load_acquire:
    ; x86 memory order does enforce the memory order for acquire by default
    ; Loads could be reordered but must always appear is if executed in order
    ; Stores cannot be moved behind loads
    ; -> no memory barrier needed
    mov rax, [rdi]
    ret

; CLINK void atomic_store(volatile int* value, int new_value);
; Args:
;   rdi -> value
;   rsi -> new_value
; Returns:
;   -
global atomic_store
atomic_store_relaxed:
    mov [rdi], rsi
    ret

; CLINK void atomic_store_release(volatile int* value, int new_value);
; Args:
;   rdi -> value
;   rsi -> new_value
; Returns:
;   -
global atomic_store_release
atomic_store_release:
    ; x86 memory order does also enforce release operation naturally similar to acquire
    ; Loads can only be moved ahead of writes not after
    ; Stores always happen in order
    ; -> no memory barrier needed
    mov [rdi], rsi
    ret

; CLINK auto atomic_test_and_set(volatile int* value, int new_value) -> int;
; Args:
;   rdi -> value
;   rsi -> new_value
; Returns:
;   The value at the memory address before it was set.
global atomic_test_and_set
atomic_test_and_set:
    mov rax, rsi
    lock xchg [rdi], rax
    ret


; CLINK auto atomic_compare_exchange_relaxed(volatile int* atomic_value, int expected, int desired) -> bool;
; Args:
;   rdi -> atomic_value
;   rsi -> expected
;   rdx -> desired
; Returns:
;   True if atomic_value, false otherwise.
global atomic_compare_exchange_relaxed
atomic_compare_exchange_relaxed:
    mov rax, rsi
    lock cmpxchg [rdi], rdx
    setz al
    ret


; CLINK auto atomic_compare_exchange_acquire(volatile int* atomic_value, int expected, int desired) -> bool;
; Args:
;   rdi -> atomic_value
;   rsi -> expected
;   rdx -> desired
; Returns:
;   True if atomic_value, false otherwise.
global atomic_compare_exchange_acquire
atomic_compare_exchange_acquire:
    ; x86 has acquire memory order by default -> see atomic_store_acquire comment for explanation
    mov rax, rsi
    lock cmpxchg [rdi], rdx
    setz al
    ret




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
    
    
; CLINK auto atomic_load(int* counter) -> int;
; Args:
;   rdi -> counter
; Returns:
;   -
global atomic_load
atomic_load:
    mov rax, [rdi]
    lfence
    ret