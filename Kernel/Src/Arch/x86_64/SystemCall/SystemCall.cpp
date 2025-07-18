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

#include <SystemCall/SystemCall.h>

#include <Hammer/Collection.h>
#include <Hammer/Algorithm.h>

#include "../CPU/X64Core.h"


namespace Rune::SystemCall {
    constexpr char const* FILE = "SystemCall";

    struct SystemCallContainer {
        SystemCallInfo info             = { 0, "", 0 };
        Handler        sys_call_handler = SYS_CALL_HANDLER_NONE;
        void* context = nullptr;
    };


    HashMap<U16, SystemCallContainer> SYSTEM_CALL_HANDLER_TABLE;
    KernelGuardian* K_GUARD;
    SharedPointer<LibK::Logger> LOGGER;



    /**
     * @brief On "syscall" the CPU will jump to this assembly stub. It loads the kernel stack and calls 
     *          system_call_dispatch. Upon return from system_call_dispatch, it will switch back to the user stack 
     *          and call "o64 sysret".
     */
    CLINK void system_call_accept();


    CLINK U64 system_call_dispatch(U16 handle, U64 arg1, U64 arg2, U64 arg3, U64 arg4, U64 arg5, U64 arg6) {
        U64  ret            = -1;
        auto handler        = SYSTEM_CALL_HANDLER_TABLE.find(handle);
        if (handler != SYSTEM_CALL_HANDLER_TABLE.end()) {
            LOGGER->trace(
                    FILE,
                    R"(Handling system call request: "{}-{}"!)",
                    handle,
                    handler->value->info.name
            );
            handler->value->info.requested++;
            ret = handler->value->sys_call_handler(
                    forward<void*>(handler->value->context),
                    arg1,
                    arg2,
                    arg3,
                    arg4,
                    arg5,
                    arg6
            );
        } else {
            LOGGER->warn(FILE, "No system call with handle {} installed!", handle);
        }
        return ret;
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          System Call API
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    bool system_call_init(SharedPointer<LibK::Logger> logger, KernelGuardian* kGuard) {
        LOGGER                    = move(logger);
        K_GUARD                   = kGuard;
        SYSTEM_CALL_HANDLER_TABLE = HashMap<U16, SystemCallContainer>();
        // Init the model specific registers for sysret/syscall, they act as caches for important values
        // CS/SS selectors
        // syscall: CS = STAR[47:32], SS = STAR[63:48] + 8, RPL bits 48:49 are 00 as syscall goes to CPL=0
        // sysret:  CS = STAR[63:48] + 16, SS = STAR[63:48] + 8, RPL bits 48:49 are 11 as sysret goes to CPL=3
        CPU::write_msr(CPU::ModelSpecificRegister::STAR, 0x0013000800000000);

        // Contains the address of the system call handler
        // Initialized with null, must be set later to correct address
        CPU::write_msr(CPU::ModelSpecificRegister::LSTAR, (uintptr_t) &system_call_accept);

        // Syscall flag mask specifies which rflags bits are to be cleared during a syscall
        // If a bit here is set to one, the rflags bit is cleared
        // If a bit here is set to zero, the rflags bit is set
        // This mask will clear all rflags bits except bit 1 which is reserved and always 1
        // Importantly it deactivates interrupts during a syscall!
        CPU::write_msr(CPU::ModelSpecificRegister::FMASK, 0xFFFFFFFFFFFFFFFD);

        // Enable the syscall and sysret instructions
        CPU::Register efer = CPU::read_msr(CPU::ModelSpecificRegister::EFER);
        CPU::write_msr(CPU::ModelSpecificRegister::EFER, set_bit(efer, 0));
        return true;
    }


    LinkedList<SystemCallInfo> system_call_get_table() {
        LinkedList<SystemCallInfo> sys_call_table;
        for (auto& sys_con: SYSTEM_CALL_HANDLER_TABLE)
            sys_call_table.add_back(sys_con.value->info);
        return sys_call_table;
    }


    bool system_call_install(const Definition& sys_call_def) {
        if (SYSTEM_CALL_HANDLER_TABLE.find(sys_call_def.handle) != SYSTEM_CALL_HANDLER_TABLE.end()) {
            LOGGER->warn(FILE, "Cannot install system call {}. It is already installed...", sys_call_def.handle);
            return false;
        }
        LOGGER->trace(
                FILE,
                R"(Installing system call "{}-{}".)",
                sys_call_def.handle,
                sys_call_def.name
        );
        SYSTEM_CALL_HANDLER_TABLE.put(
                sys_call_def.handle,
                {
                        { sys_call_def.handle, sys_call_def.name, 0 },
                        sys_call_def.sys_call_handler,
                        sys_call_def.context
                }
        );
        return true;
    }


    bool system_call_uninstall(U16 system_call_handle) {
        if (SYSTEM_CALL_HANDLER_TABLE.find(system_call_handle) == SYSTEM_CALL_HANDLER_TABLE.end()) {
            LOGGER->trace(FILE, "System call {} is not installed. No need to uninstall...", system_call_handle);
            return false;
        }

        auto sys_call = SYSTEM_CALL_HANDLER_TABLE.find(system_call_handle);
        LOGGER->trace(
                FILE,
                R"(Uninstalling system call "{}-{}".)",
                sys_call->value->info.handle,
                sys_call->value->info.name
        );
        return SYSTEM_CALL_HANDLER_TABLE.remove(system_call_handle);
    }
}