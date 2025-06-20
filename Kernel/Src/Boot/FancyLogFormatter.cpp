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

#include <Boot/FancyLogFormatter.h>


namespace Rune {
    FancyLogFormatter::FancyLogFormatter(CPU::Subsystem* cpu_subsys, App::Subsystem* app_subsys)
            : _cpu_subsys(cpu_subsys),
              _app_subsys(app_subsys) {

    }


    String FancyLogFormatter::format_log_message(
            LibK::LogLevel log_level,
            const String& module,
            const String& log_msg_tmpl,
            Argument* arg_list,
            size_t arg_size
    ) {
        auto r_thread = _cpu_subsys->get_scheduler()->get_running_thread();
        App::Info* r_app = _app_subsys->get_active_app();
        return String::format("[{}] [{}] [{}] [{}] ", log_level.to_string(), module, r_app->name, r_thread->name) +
               String::format(log_msg_tmpl, arg_list, arg_size);
    }
}
