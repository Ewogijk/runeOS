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

; CLINK void out_b(U16 port, U8 value);
; Args:
;   rdi -> port
;   rsi -> value
; Returns:
;   -
global out_b
out_b:
    push rbp
    mov rbp, rsp

    mov dx, di
    mov al, sil
    out dx, al

    mov rsp, rbp
    pop rbp
    ret


; CLINK U8 in_b(U16 port);
; Args:
;   rdi -> port
; Returns:
;   Byte from the port.
global in_b
in_b:
    push rbp
    mov rbp, rsp

    mov dx, di
    xor rax, rax
    in al, dx

    mov rsp, rbp
    pop rbp
    ret


; CLINK void out_w(U16 port, U16 value);
; Args:
;   rdi -> port
;   rsi -> value
; Returns:
;   -
global out_w
out_w:
    push rbp
    mov rbp, rsp

    mov dx, di
    mov ax, si
    out dx, ax

    mov rsp, rbp
    pop rbp
    ret


; CLINK U16 in_w(U16 port);
; Args:
;   rdi -> port
; Returns:
;   Word from the port.
global in_w
in_w:
    push rbp
    mov rbp, rsp

    mov dx, di
    xor rax, rax
    in ax, dx

    mov rsp, rbp
    pop rbp
    ret


; CLINK void out_dw(U16 port, U32 value);
; Args:
;   rdi -> port
;   rsi -> value
; Returns:
;   -
global out_dw
out_dw:
    push rbp
    mov rbp, rsp

    mov dx, di
    mov eax, esi
    out dx, eax

    mov rsp, rbp
    pop rbp
    ret


; CLINK U32 in_dw(U16 port);
; Args:
;   rdi -> port
; Returns:
;   Double word from the port
global in_dw
in_dw:
    push rbp
    mov rbp, rsp

    mov dx, di
    xor rax, rax
    in eax, dx

    mov rsp, rbp
    pop rbp
    ret


; CLINK void io_wait();
; Args:
;   -
; Returns:
;   -
global io_wait
io_wait:
    push rbp
    mov rbp, rsp

    mov al, 0x00
    out 0x80, al

    mov rsp, rbp
    pop rbp
    ret