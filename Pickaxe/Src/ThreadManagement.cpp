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

#include <Pickaxe/ThreadManagement.h>


namespace Rune::Pickaxe {

    int mutex_create(const char* mutex_name) {
        return system_call1(300, (uintptr_t) mutex_name);
    }


    int mutex_lock(U16 handle) {
        return system_call1(301, handle);
    }


    int mutex_unlock(U16 handle) {
        return system_call1(302, handle);
    }


    int mutex_release(U16 handle) {
        return system_call1(303, handle);
    }
}