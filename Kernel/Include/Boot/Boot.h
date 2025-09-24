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

#ifndef RUNEOS_BOOT_H
#define RUNEOS_BOOT_H

#include <KRE/System/Subsystem.h>

namespace Rune {
    /**
     * Entry point to the high level boot routine.
     * <p>
     *  Following features need to be initialized by the low level kernel:
     *  <ul>
     *    <li>Global constructors.</li>
     *    <li>CPU specific features.</li>
     *    <li>Physical memory map.</li>
     *    <li>Linear framebuffer.</li>
     *  </ul>
     * </p>
     *
     * @param boot_loader_info Information provided by the boot loader.
     */
    void kernel_boot(BootLoaderInfo boot_loader_info);
} // namespace Rune

#endif // RUNEOS_BOOT_H
