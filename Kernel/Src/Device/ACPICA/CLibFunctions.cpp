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

#include <Ember/Ember.h>

#include <KRE/String.h>

CLINK {
    // ========================================================================================== //
    // C Library functions required by ACPICA
    // ========================================================================================== //

    using namespace Rune;

    size_t strlen(const char* str) { return String(str).size(); }

    char* strcat(char* destination, const char* source) {
        memcpy(&destination[strlen(destination)], source, strlen(source));
        return destination;
    }

    char* strncat(char* destination, const char* source, size_t num) {
        memcpy(&destination[strlen(destination)], source, num);
        return destination;
    }

    char* strcpy(char* destination, const char* source) {
        memcpy(destination, source, strlen(source));
        return destination;
    }

    char* strncpy(char* destination, const char* source, size_t num) {
        memcpy(destination, source, num);
        return destination;
    }

    int strncmp(const char* str1, const char* str2, size_t num) {
        char* c1 = const_cast<char*>(str1);
        char* c2 = const_cast<char*>(str2);
        int   i  = 0;
        while (c1 && c2 && i < num) {
            if (*c1 != *c2) return c1 < c2 ? -1 : 1;
            c1++;
            c2++;
            i++;
        }
        if (c1 != c2) return c1 < c2 ? -1 : 1;
        return 0;
    }

    int strcmp(const char* str1, const char* str2) {
        char* c1 = const_cast<char*>(str1);
        char* c2 = const_cast<char*>(str2);
        while (c1 && c2) {
            if (*c1 != *c2) return c1 < c2 ? -1 : 1;
            c1++;
            c2++;
        }
        if (c1 != c2) return c1 < c2 ? -1 : 1;
        return 0;
    }

    int isdigit(int c) { return '0' <= c && c <= '9'; }

    int isspace(int c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n' | c == '\v' | c == '\f';
    }

    int isxdigit(int c) {
        return 0x30 <= c && c <= 0x39 || 0x41 <= c && c <= 0x46 || 0x61 <= c && c <= 0x66;
    }

    int isprint(int c) { return 0x20 <= c && c <= 0x7F; }

    int tolower(int c) { return String(static_cast<char>(c)).lower()[0]; }

    int toupper(int c) { return String(static_cast<char>(c)).upper()[0]; }
}
