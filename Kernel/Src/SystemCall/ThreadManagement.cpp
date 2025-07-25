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

#include <SystemCall/ThreadManagement.h>


namespace Rune::SystemCall {

    S64 mutex_create(void* sys_call_ctx, U64 mutex_name) {
        auto* t_ctx = (ThreadManagementContext*) sys_call_ctx;

        char km_name[KernelGuardian::USER_STRING_LIMIT];
        memset(km_name, 0, KernelGuardian::USER_STRING_LIMIT);
        if (!t_ctx->k_guard->copy_string_user_to_kernel((const char*) mutex_name, -1, km_name))
            return -1;

        String m_name(km_name);
        if (m_name.is_empty())
            m_name = String::format("Mutex-App{}", t_ctx->app_subsys->get_active_app()->handle);

        auto m = t_ctx->cpu_subsys->create_mutex(m_name);
        if (!m)
            return -2;
        return m->handle;
    }


    S64 mutex_lock(void* sys_call_ctx, U64 handle) {
        auto* t_ctx = (ThreadManagementContext*) sys_call_ctx;
        if (handle == 0)
            return -1;

        auto m = t_ctx->cpu_subsys->find_mutex(handle);
        if (!m)
            return -2;

        m->lock();
        return 0;
    }


    S64 mutex_unlock(void* sys_call_ctx, U64 handle) {
        auto* t_ctx = (ThreadManagementContext*) sys_call_ctx;
        if (handle == 0)
            return -1;

        auto m = t_ctx->cpu_subsys->find_mutex(handle);
        if (!m)
            return -2;

        m->unlock();
        return 0;
    }


    S64 mutex_release(void* sys_call_ctx, U64 handle) {
        auto* t_ctx = (ThreadManagementContext*) sys_call_ctx;
        if (handle == 0)
            return -1;

        if (!t_ctx->cpu_subsys->release_mutex(handle))
            return -2;

        return 0;
    }


    S64 get_thread_ID(void* sys_call_ctx) {
        auto* t_ctx      = (ThreadManagementContext*) sys_call_ctx;
        return t_ctx->cpu_subsys->get_scheduler()->get_running_thread()->handle;
    }


    S64 get_thread_control_block(void* sys_call_ctx, U64 tcb_out) {
        auto* t_ctx      = (ThreadManagementContext*) sys_call_ctx;
        auto* tcb_out_ptr = LibK::memory_addr_to_pointer<ThreadControlBlock>(tcb_out);
        if (!t_ctx->k_guard->verify_user_buffer(tcb_out_ptr, sizeof(ThreadControlBlock)))
            return -1;

        auto c_t = t_ctx->cpu_subsys->get_scheduler()->get_running_thread();
        tcb_out_ptr->thread_ID = c_t->handle;
        tcb_out_ptr->stack_addr = c_t->user_stack.stack_bottom;
        tcb_out_ptr->stack_size = c_t->user_stack.stack_size;
        return 0;
    }


    S64 set_thread_control_block(void* sys_call_ctx, const U64 tcb) {
        const auto* t_ctx   = static_cast<ThreadManagementContext*>(sys_call_ctx);
        auto* tcb_ptr = LibK::memory_addr_to_pointer<void>(tcb);
        if (!t_ctx->k_guard->verify_user_buffer(tcb_ptr, sizeof(ThreadControlBlock)))
            return -1;
        t_ctx->cpu_subsys->get_scheduler()->get_running_thread()->thread_control_block = tcb_ptr;
        CPU::current_core()->update_thread_local_storage(tcb_ptr);
        return 0;
    }
}