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

#include <Hammer/Memory.h>


void *memset(void *dest, int ch, size_t count)
{
    auto *d = (unsigned char *) dest;

    for (size_t i = 0; i < count; i++)
        d[i] = ch;
    return dest;
}

void *memcpy(void *dest, void *src, size_t count)
{
    auto *d = (unsigned char *) dest;
    auto *s = (unsigned char *) src;

    for (size_t i = 0; i < count; i++)
        d[i] = s[i];

    return dest;
}

void *memmove(void *dest, void *src, size_t count)
{
    uintptr_t sourceEnd = (uintptr_t) src + count;
    auto      *d        = (unsigned char *) dest;
    auto      *s        = (unsigned char *) src;
    if ((uintptr_t) dest <= sourceEnd && sourceEnd <= (uintptr_t) dest + count)
    {
        // Source overlaps from left -> Copy from end
        //     dddddd
        //  ssssss
        for (size_t i = count; i > 0; i--)
            d[i - 1] = s[i - 1];
    } else
    {
        // Source overlaps from right or no overlap -> Copy from start
        //    ssssss
        // dddddd
        // or
        // ssssss
        //        dddddd
        for (size_t i = 0; i < count; i++)
            d[i] = s[i];
    }
    return dest;
}

int memcmp(const void *lhs, const void *rhs, size_t count)
{
    auto *l = (unsigned char *) lhs;
    auto *r = (unsigned char *) rhs;

    for (size_t i = 0; i < count; i++)
    {
        if (l[i] < r[i])
            return -1;
        else if (l[i] > r[i])
            return 1;
    }
    return 0;
}