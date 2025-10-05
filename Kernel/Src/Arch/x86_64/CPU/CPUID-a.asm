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

; CLINK bool cpuid_is_supported();
; Args:
;   -
;
global cpuid_is_supported
cpuid_is_supported:
    push rbp
    mov rbp, rsp

    pushfq                  ; Save original rflags

    pushfq                  ; Store rflags for modification
    mov rax, [rsp]          ; Invert ID bit
    xor rax, 0x00200000
    mov [rsp], rax
    popfq                   ; Load modified rflags

    pushfq                  ; Save modified rflags
    pop rax
    xor rax, [rsp]          ; rax contains all modified bits

    popfq                   ; Restore original rflags

    and rax, 0x00200000     ; Check if ID bit (position 21) is modified
    shr rax, 21

    leave
    ret


; CLINK void cpuid_make_request(U64 request, CPUIDResponse* resp);
; Args:
;   rdi -> request
;   rsi -> resp
; Returns:
;   -
global cpuid_make_request
cpuid_make_request:
    ; make new call frame
    push rbp            ; save old call frame
    mov rbp, rsp        ; initialize new call frame

    push rbx

    mov rax, rdi
    cpuid

    mov [rsi], rax
    mov [rsi + 8], rbx
    mov [rsi + 16], rcx
    mov [rsi + 24], rdx

    pop rbx

    ; restore old call frame
    mov rsp, rbp
    pop rbp
    ret