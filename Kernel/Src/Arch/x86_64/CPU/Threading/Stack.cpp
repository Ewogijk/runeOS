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

#include <CPU/Threading/Stack.h>


namespace Rune::CPU {
    VirtualAddr setup_empty_stack(VirtualAddr stack_top) {
        auto* s_top = reinterpret_cast<U64*>(stack_top);
        // Push the null frame marking the end of the stack
        *(--s_top) = 0;

        return memory_pointer_to_addr(s_top);
    }


    VirtualAddr setup_trampoline_kernel_stack(
            VirtualAddr stack_top,
            VirtualAddr thread_enter
    ) {
        // Set up the stack so that the CPU jumps to the thread_enter function on context switch with a null frame at
        // the bottom.
        //
        // thread_enter signature: void ThreadEnter();
        //
        // Additionally we need to push zeroes for all preserved registers as defined in context_switch_ass()
        // in X64Core-a.asm onto the stack, check it out for details.
        //
        // ------------------------------------------
        // |       Initial Stack Layout             |
        // ------------------------------------------
        // |      0                 (Return Addr)   | <- Null Frame
        // |      thread_enter      (Return Addr)   |
        // |      0                 (RBX)           | <-- RBP
        // |      Stack Top - 24    (RBP)           |
        // |      0                 (R12)           |
        // |      0                 (R13)           |
        // |      0                 (R14)           |
        // |      0                 (R15)           |
        // |      0                 (XMM0-hi)       |
        // |      0                 (XMM0-lo)       |
        // |                    .                   |
        // |                    .                   |
        // |                    .                   |
        // |      0                 (XMM15-hi)      |
        // |      0                 (XMM15-lo)      | <-- RSP
        // ------------------------------------------

        auto* s_top = reinterpret_cast<U64*>(stack_top);
        // Push the null frame
        *(--s_top) = 0;

        // Init stack frame
        *(--s_top) = thread_enter;      // Return addr
        *(--s_top) = 0;                 // RBX
        *(--s_top) = stack_top - 24;    // RBP
        *(--s_top) = 0;                 // R12
        *(--s_top) = 0;                 // R13
        *(--s_top) = 0;                 // R14
        *(--s_top) = 0;                 // R15

        // XMM0-XMM15, 16 registers 128bit wide. but we can only push 64 bit values
        // therefore push 2*16 zeroes
        for (int i = 0; i < 32; i++)
            *(--s_top) = 0;

        return memory_pointer_to_addr(s_top);
    }
}
