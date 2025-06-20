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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                          Paging Configuration
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; CLINK void load_base_page_table(PhysicalAddr base_pt);
; Args:
;   rdi -> base_pt
; Returns:
;   -
global load_base_page_table
load_base_page_table:
    mov cr3, rax
    ret


; CLINK void invalidate_page(VirtualAddr page);
; Args:
;   rdi -> page
;   rsi ->
;   rdx ->
;   rcx ->
; Returns:
;   -
global invalidate_page
invalidate_page:
    invlpg [rdi]
    ret


; CLINK void flush_tlb();
; Args:
;   rdi ->
;   rsi ->
;   rdx ->
;   rcx ->
; Returns:
;   -
global flush_tlb
flush_tlb:
    ; Reloading cr3 will flush all TBL entries
    mov rax, cr3
    mov cr3, rax
    ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                          Page Table Hierarchy Access
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; CLINK PhysicalAddr get_base_page_table_address();
; Args:
;   rdi ->
;   rsi ->
;   rdx ->
;   rcx ->
; Returns:
;   -
global get_base_page_table_address
get_base_page_table_address:
    mov rax, cr3
    ret