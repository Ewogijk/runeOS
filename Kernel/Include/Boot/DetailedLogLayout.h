
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

#ifndef RUNEOS_DETAILEDLOGLAYOUT_H
#define RUNEOS_DETAILEDLOGLAYOUT_H

#include <KRE/Logging.h>

#include <CPU/CPUSubsystem.h>

#include <App/AppSubsystem.h>

namespace Rune {
    class DetailedLogLayout : public Layout {
        CPU::CPUSubsystem* _cpu_subsys;
        App::AppSubsystem* _app_subsys;

      public:
        DetailedLogLayout(CPU::CPUSubsystem* cpu_subsys, App::AppSubsystem* app_subsys);

        auto layout(const LogEvent& log_event) -> String override;
    };
} // namespace Rune

#endif // RUNEOS_DETAILEDLOGLAYOUT_H
