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

#include <CPU/CPU.h>

#include "X64Core.h"

#include <KRE/Collections/LinkedList.h>

namespace Rune::CPU {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Core API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // The bootstrap core is the initially running core when the device is powered on
    // We need to declare it globally because we cannot allocate it on the kernel heap
    // this early, since the core init is the first thing we do after the bootloader gives
    // control to us
    X64Core           BOOTSTRAP_CORE = X64Core(0);
    LinkedList<Core*> CORES;

    bool init_bootstrap_core() { return BOOTSTRAP_CORE.init(); }

    bool init_other_cores() {
        CORES.add_back(&BOOTSTRAP_CORE);
        return true;
    }

    Core* current_core() { return &BOOTSTRAP_CORE; }

    LinkedList<Core*> get_core_table() { return CORES; }
} // namespace Rune::CPU
