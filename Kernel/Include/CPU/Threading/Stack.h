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

#ifndef RUNEOS_STACK_H
#define RUNEOS_STACK_H

#include <KRE/Memory.h>

namespace Rune::CPU {

    /**
     * @brief Push a null frame to the stack to allow stack tracing.
     * @param stack_top Top of the stack.
     * @return The new top of the stack.
     */
    auto setup_empty_stack(VirtualAddr stack_top) -> VirtualAddr;

    /**
     * @brief Setup the kernel stack so that it returns to the provided thread_enter function with
     * the per ABI preserved registers initialized to zero.
     *
     * A null frame will pushed to the bottom of the stack to allow stack tracing.
     *
     * @param stack_top    Top of the kernel stack.
     * @param thread_enter Virtual address of the Thread Enter function that prepares a thread for
     * execution.
     * @return The new top of the kernel stack.
     */
    auto setup_trampoline_kernel_stack(VirtualAddr stack_top, VirtualAddr thread_enter) -> VirtualAddr;

} // namespace Rune::CPU

#endif // RUNEOS_STACK_H
