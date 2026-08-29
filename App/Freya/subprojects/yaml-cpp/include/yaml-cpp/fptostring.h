//  Copyright 2026 Ewogijk
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

#ifndef YAML_H_FPTOSTRING
#define YAML_H_FPTOSTRING

#include "yaml-cpp/dll.h"

#include <string>

namespace YAML {
// "precision = 0" refers to shortest known unique representation of the value
YAML_CPP_API std::string FpToString(float v, size_t precision = 0);
YAML_CPP_API std::string FpToString(double v, size_t precision = 0);
YAML_CPP_API std::string FpToString(long double v, size_t precision = 0);
}

#endif
