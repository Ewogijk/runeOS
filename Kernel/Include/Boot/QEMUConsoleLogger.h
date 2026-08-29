
//  Copyright 2026 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef RUNEOS_QEMUCONSOLELOGGER_H
#define RUNEOS_QEMUCONSOLELOGGER_H

namespace Rune {

    /// @brief Start a background thread that polls the kernel ring buffer periodically and forwards
    ///        formatted log events to the QEMU debug port.
    void qemu_consoler_logger_start();
} // namespace Rune

#endif // RUNEOS_QEMUCONSOLELOGGER_H
