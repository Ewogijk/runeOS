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
global interrupt_irq_enable
interrupt_irq_enable:
    sti
    ret

; CLINK void interrupt_disable();
global interrupt_irq_disable
interrupt_irq_disable:
    cli
    ret

; CLINK auto interrupt_irq_save() -> Register;
; Args:
;   rdi -> -
;   rsi -> -
;   rdx -> -
;   rcx -> -
; Returns:
;   Flags register content before disabling interrupts.
global interrupt_irq_save
interrupt_irq_save:
    enter 0, 0

    pushfq
    pop rax
    cli

    leave
    ret

; CLINK void interrupt_irq_restore(Register flags);
; Args:
;   rdi -> Flags register content saved previously.
;   rsi -> -
;   rdx -> -
;   rcx -> -
; Returns:
;   -
global interrupt_irq_restore
interrupt_irq_restore:
    enter 0, 0

    push rdi
    popfq
    sti

    leave
    ret


