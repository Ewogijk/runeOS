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

; CLINK void interrupt_enable();
global interrupt_enable
interrupt_enable:
    push rbp
    mov rbp, rsp

    sti

    mov rsp, rbp
    pop rbp
    ret

; CLINK void interrupt_disable();
global interrupt_disable
interrupt_disable:
    push rbp
    mov rbp, rsp

    cli

    mov rsp, rbp
    pop rbp
    ret