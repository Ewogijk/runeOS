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

; CLINK void load_gdtr(GlobalDescriptorTable* gdt, U16 code_segment, U16 data_segment);
; Args:
;   rdi -> gdt
;   rsi -> code_segment
;   rdi -> data_segment
; Return:
;   -
global load_gdtr
load_gdtr:
    ; make new call frame
    push rbp            ; save old call frame
    mov rbp, rsp        ; initialize new call frame

    ; load gdt
    lgdt [rdi]

    ; reload code segment
    push rsi
    lea rax, [rel .reload_cs]
    push rax
    retfq

.reload_cs:
    ; reload data segments
    mov ax, dx
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; restore old call frame
    mov rsp, rbp
    pop rbp
    ret

;CLINK void load_task_state_register(TaskStateSegment64* tss_offset);
; Args:
;   rdi -> tss_offset
; Return:
;   -
global load_task_state_register
load_task_state_register:
    ; make new call frame
    push rbp            ; save old call frame
    mov rbp, rsp        ; initialize new call frame

    mov ax, di
    ltr ax

    ; restore old call frame
    mov rsp, rbp
    pop rbp
    ret