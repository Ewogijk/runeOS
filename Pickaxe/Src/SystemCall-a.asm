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

; extern "C" int make_system_call(SystemCallPayload* sys_call_payload);
; Args:
;   rdi -> sys_call_payload
; Returns:
;   The return value from the kernel.
global make_system_call
make_system_call:
    ; make new call frame
    push rbp            ; save old call frame
    mov rbp, rsp        ; initialize new call frame

    ; We use the System V ABI for system calls so we do not have to do anything here
    syscall

    ; restore old call frame
    mov rsp, rbp
    pop rbp
    ret