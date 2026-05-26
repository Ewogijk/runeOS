
//  Copyright 2025 Ewogijk
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

#ifndef RUNEOS_CRL_H
#define RUNEOS_CRL_H

#include <Ember/Ember.h>
#include <contracts>

// ============================================================================================== //
// C/C++ Compiler expects these to be defined
// ============================================================================================== //

CLINK auto memset(void* dest, int chr, size_t count) -> void*;

CLINK auto memcpy(void* dest, const void* src, size_t count) -> void*;

CLINK auto memmove(void* dest, const void* src, size_t count) -> void*;

CLINK auto memcmp(const void* lhs, const void* rhs, size_t count) -> int;

// ============================================================================================== //
// New / Delete operators
// Note: Implementations must be provided by the Kernel or Userspace
// ============================================================================================== //

auto operator new(size_t size) -> void*;

auto operator new[](size_t size) -> void*;

// Non-allocating placement allocation
auto operator new(size_t count, void* ptr) -> void*;

auto operator new[](size_t count, void* ptr) -> void*;

void operator delete(void* ptr) noexcept;

void operator delete(void* ptr, size_t size) noexcept;

void operator delete[](void* ptr) noexcept;

void operator delete[](void* ptr, size_t size) noexcept;

// ============================================================================================== //
// Will be called when there is no implementation of a pure virtual function
// (should not happen because compiler cries)
// ============================================================================================== //

CLINK void __cxa_pure_virtual(); // NOLINT

// ============================================================================================== //
// Stack smash protection
// ============================================================================================== //

CLINK void __stack_chk_fail(void); // NOLINT

// ============================================================================================== //
// Catch all section
// ============================================================================================== //

/// Normally: Register a function to be called at normal program termination.
/// But we simply implement the function stub because the compiler requires this function when
/// static local variables are declared.
/// @param func
/// @return
auto atexit(void (*func)()) -> int;

// ============================================================================================== //
// Contracts
// ============================================================================================== //

namespace std {
    auto terminate() -> void;
} // namespace std

auto handle_contract_violation(std::contracts::contract_violation const&) -> void;

// ============================================================================================== //
// CRL Initialization
// ============================================================================================== //

namespace Rune {
    /// @brief Init the C++ runtime layer.
    /// @param on_cxa_pure_virtual Callback for "__cxa_pure_virtual"
    /// @param on_stack_guard_fail Callback for "__stack_chk_fail"
    /// @param on_terminate Callback for "std::terminate"
    /// @param on_handle_contract_violation Callback for "handle_contract_violation"
    void init_cpp_runtime_layer(
        void (*on_cxa_pure_virtual)(),
        void (*on_stack_guard_fail)(),
        void (*on_terminate)(),
        void (*on_handle_contract_violation)(std::contracts::contract_violation const&));
} // namespace Rune

#endif // RUNEOS_CRL_H
