/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

ENTRY(_start)

PHDRS
{
    /* flags bits: 2: execute, 1: write, 0: read */
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ;
    rodata  PT_LOAD    FLAGS((1 << 2)) ;
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ;
}

SECTIONS
{
    /* We load the app at 1 GiB address because why not and this is user space*/
    . = 0x40000000;

    .text : {
        *(.text*)
    } :text

    /* Move the location counter to the next page boundary */
    . += CONSTANT(MAXPAGESIZE);

    .rodata : {
        *(SORT(.ctors*))            /* C++: Constructors for global/static objects */
        *(SORT(.dtors*))            /* C++: Destructors for global/static objects */
        *(.rodata*)
        *(.eh_frame)                /* Required for crtbegin.o */
    } :rodata

    . += CONSTANT(MAXPAGESIZE);

    .data : {
        *(.data*)
    } :data

    .bss : {
        *(.bss*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.eh_frame)                /* Discard call frame information - Needed for exceptions */
        *(.note*)
    }
}
