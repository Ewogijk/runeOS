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

#include <Boot/DetailedLogLayout.h>

namespace Rune {
    DetailedLogLayout::DetailedLogLayout(CPU::CPUSubsystem* cpu_subsys,
                                         App::AppSubsystem* app_subsys)
        : _cpu_subsys(cpu_subsys),
          _app_subsys(app_subsys) {}

    auto DetailedLogLayout::layout(const LogEvent& log_event) -> String {
        auto       r_thread = _cpu_subsys->get_scheduler()->get_running_thread();
        App::Info* r_app    = _app_subsys->get_active_app();
        return String::format("[{}] [{}] [{}] [{}] ",
                              log_event.log_level.to_string(),
                              log_event.logger_name,
                              r_app->name,
                              r_thread->name)
               + String::format(log_event.log_msg_template,
                                static_cast<const Argument*>(log_event.arg_list),
                                log_event.arg_size);
    }

} // namespace Rune
