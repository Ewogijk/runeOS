/*Heimdall
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

#include <Test/Heimdall/Heimdall.h>

#include <Test/IntegrationTest/tests/Dummy.h>

int main(const int argc, char* argv[]) {
    Heimdall::OptionList options;
    options.insert({.name = Heimdall::Engine::CONSOLE_REPORTER, .value = ""});
    options.insert({.name  = Heimdall::Engine::TEST_REPORT_LOCATION,
                    .value = "/System/Heimdall/IntegrationTestReport.txt"});
    Heimdall::execute_tests(options);
    return 0;
}
