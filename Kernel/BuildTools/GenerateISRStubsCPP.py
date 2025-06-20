#!/usr/bin/env python3

#   Copyright 2025 Ewogijk
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import click


@click.command('isr-stubs-cpp')
@click.argument('out_file', type=str)
def generate_isr_cpp_stubs(out_file: str) -> None:
    """Generate the ISR_Stubs.cpp source file.

    :param out_file: Path to the ISR_Stubs.cpp file.
    :return: -
    """
    with open(out_file, 'w') as file:
        file.write('/*\n')
        file.write(' *  Copyright 2025 Ewogijk\n')
        file.write(' *\n')
        file.write(' *  Licensed under the Apache License, Version 2.0 (the "License");\n')
        file.write(' *  you may not use this file except in compliance with the License.\n')
        file.write(' *  You may obtain a copy of the License at\n')
        file.write(' *\n')
        file.write(' *      http://www.apache.org/licenses/LICENSE-2.0\n')
        file.write(' *\n')
        file.write(' *  Unless required by applicable law or agreed to in writing, software\n')
        file.write(' *  distributed under the License is distributed on an "AS IS" BASIS,\n')
        file.write(' *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n')
        file.write(' *  See the License for the specific language governing permissions and\n')
        file.write(' *  limitations under the License.\n')
        file.write(' */\n\n')

        file.write('/*\n')
        file.write(' * This file is auto generated.\n')
        file.write(' */\n\n')

        file.write(f'#ifndef RUNEOS_ISR_STUBS_H \n')
        file.write(f'#define RUNEOS_ISR_STUBS_H\n\n')

        file.write('#include "IDT.h"\n\n')

        for i in range(0, 256):
            file.write(f'CLINK void ISR{i}();\n')

        file.write('namespace Rune::CPU {\n')

        file.write('    void init_interrupt_service_routines() {\n')
        file.write('        U8 gdt_offset = 0x08;\n')
        for i in range(0, 256):
            file.write(
                f'        idt_set({i}, (void*) ISR{i}, gdt_offset, 0, GateType::INTERRUPT_GATE, 0, false);\n')
        file.write('    }\n')

        file.write('}\n\n')

        file.write(f'#endif //RUNEOS_ISR_STUBS_H')


if __name__ == '__main__':
    generate_isr_cpp_stubs()
