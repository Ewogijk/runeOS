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

#include <SystemCall/ThreadingBundle.h>

#include <Ember/StatusCode.h>


namespace Rune::SystemCall {
    Ember::StatusCode mutex_create(void* sys_call_ctx, const U64 mutex_name) {
        const auto* t_ctx = static_cast<ThreadingSystemCallContext*>(sys_call_ctx);

        char km_name[Ember::STRING_SIZE_LIMIT] = {};
        if (!t_ctx->k_guard->copy_string_user_to_kernel(reinterpret_cast<const char*>(mutex_name), -1, km_name))
            return Ember::Status::BAD_ARG;

        String m_name(km_name);
        if (m_name.is_empty())
            m_name = String::format("Mutex-App{}", t_ctx->app_subsys->get_active_app()->handle);

        const auto m = t_ctx->cpu_subsys->create_mutex(m_name);
        return m ? m->handle : Ember::Status(Ember::Status::FAULT).to_value();
    }


    Ember::StatusCode mutex_lock(void* sys_call_ctx, const U64 ID) {
        const auto* t_ctx = static_cast<ThreadingSystemCallContext*>(sys_call_ctx);
        if (ID == 0)
            return Ember::Status::BAD_ARG;

        const auto m = t_ctx->cpu_subsys->find_mutex(ID);
        if (!m)
            return Ember::Status::UNKNOWN_ID;

        m->lock();
        return Ember::Status::OKAY;
    }


    Ember::StatusCode mutex_unlock(void* sys_call_ctx, const U64 ID) {
        const auto* t_ctx = static_cast<ThreadingSystemCallContext*>(sys_call_ctx);
        if (ID == 0)
            return Ember::Status::BAD_ARG;

        const auto  m = t_ctx->cpu_subsys->find_mutex(ID);
        if (!m)
            return Ember::Status::UNKNOWN_ID;

        m->unlock();
        return Ember::Status::OKAY;
    }


    Ember::StatusCode mutex_free(void* sys_call_ctx, const U64 ID) {
        const auto* t_ctx = static_cast<ThreadingSystemCallContext*>(sys_call_ctx);
        if (ID == 0)
            return Ember::Status::BAD_ARG;

        return t_ctx->cpu_subsys->release_mutex(ID) ? Ember::Status::OKAY : Ember::Status::UNKNOWN_ID;
    }


    Ember::StatusCode get_thread_ID(void* sys_call_ctx) {
        const auto* t_ctx = static_cast<ThreadingSystemCallContext*>(sys_call_ctx);
        return t_ctx->cpu_subsys->get_scheduler()->get_running_thread()->handle;
    }


    Ember::StatusCode set_thread_control_block(void* sys_call_ctx, const U64 tcb) {
        const auto* t_ctx   = static_cast<ThreadingSystemCallContext*>(sys_call_ctx);
        auto*       tcb_ptr = LibK::memory_addr_to_pointer<void>(tcb);
        if (!t_ctx->k_guard->verify_user_buffer(tcb_ptr, sizeof(void*)))
            return Ember::Status::BAD_ARG;
        t_ctx->cpu_subsys->get_scheduler()->get_running_thread()->thread_control_block = tcb_ptr;
        CPU::current_core()->update_thread_local_storage(tcb_ptr);
        return Ember::Status::OKAY;
    }
}
