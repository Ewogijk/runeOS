
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

#include <CPU/CPUModule.h>

#include <App/AppModule.h>

namespace Rune {
    class DetailedLogLayout : public Layout {
        CPU::CPUModule* _cpu_module;
        App::AppModule* _app_module;

      public:
        DetailedLogLayout(CPU::CPUModule* cpu_module, App::AppModule* app_module);

        auto layout(LogLevel      log_level,
                    const String& logger_name,
                    const String& log_msg_template,
                    Argument*     arg_list,
                    size_t        arg_size) -> String override;
    };
} // namespace Rune

#endif // RUNEOS_DETAILEDLOGLAYOUT_H
