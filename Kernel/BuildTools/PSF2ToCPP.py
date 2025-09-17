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

from pathlib import Path


PSF1_MAGIC = int.from_bytes(b"\x36\x04", "little")
PSF2_MAGIC = int.from_bytes(b"\x72\xb5\x4a\x86", "little")


class BitMapFont:
    name = ""
    number_of_glyphs = 0
    glyph_size = 0
    pixel_height = 0
    pixel_width = 0
    glyphs = []


def read_psf2_file(out_file: Path) -> BitMapFont:
    """Try to parse the given psf file and generate an in-memory representation of the bit map font.

    :param out_file: File handle
    :return: A bitmap font.
    """
    with open(out_file, "rb") as psf2file:
        magic_bytes = psf2file.read(4)
        num_glyphs = 0
        glyph_size = 0
        height = 0
        width = 0

        maybe_psf2_magic = int.from_bytes(magic_bytes, "little")
        if maybe_psf2_magic == PSF2_MAGIC:
            psf2file.read(12)  # skip version, header size and flags
            num_glyphs = int.from_bytes(psf2file.read(4), "little")
            glyph_size = int.from_bytes(psf2file.read(4), "little")
            height = int.from_bytes(psf2file.read(4), "little")
            width = int.from_bytes(psf2file.read(4), "little")
        else:
            # check for psf1 magic
            maybe_psf1_magic = int.from_bytes(bytes(magic_bytes[:2]), "little")
            if maybe_psf1_magic == PSF1_MAGIC:
                flags = int(magic_bytes[2])
                num_glyphs = 512 if flags & 1 == 1 else 256
                glyph_size = height = int(magic_bytes[3])
                width = 8  # Fixed width of 8 pixels in psf1
            else:
                print(
                    f"{out_file} is not a psf1 or psf2 file! "
                    f"Expected magic: {format(PSF1_MAGIC, '#04x')} or {format(PSF2_MAGIC, '#06x')},"
                    f" Found: {format(int.from_bytes(magic_bytes, 'little'), '#010x')}"
                )
                return BitMapFont()

        # read up to 128 glyphs aka ascii glyphs
        glyphs = []
        for i in range(min(num_glyphs, 128)):
            g = [b for b in psf2file.read(glyph_size)]
            glyphs.append(g)

        # skip the unicode table, only care about ascii at the moment

        font = BitMapFont()
        font.name = Path(out_file).stem
        font.number_of_glyphs = min(num_glyphs, 128)
        font.glyph_size = glyph_size
        font.pixel_height = height
        font.pixel_width = width
        font.glyphs = glyphs

        return font


def generate_cpp_font_file(cpp_file: Path, font: BitMapFont) -> None:
    """Generate a cpp header file from the given bitmap font.

    :param cpp_file: Path to a CPP source file.
    :param font: Bitmap font.
    :return: -
    """
    font_name_cppified = font.name.replace("-", "").replace("x", "").upper()
    with open(cpp_file, "w") as file:
        file.write("/*\n")
        file.write(" *  Copyright 2025 Ewogijk\n")
        file.write(" *\n")
        file.write(' *  Licensed under the Apache License, Version 2.0 (the "License");\n')
        file.write(" *  you may not use this file except in compliance with the License.\n")
        file.write(" *  You may obtain a copy of the License at\n")
        file.write(" *\n")
        file.write(" *      http://www.apache.org/licenses/LICENSE-2.0\n")
        file.write(" *\n")
        file.write(" *  Unless required by applicable law or agreed to in writing, software\n")
        file.write(' *  distributed under the License is distributed on an "AS IS" BASIS,\n')
        file.write(" *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n")
        file.write(" *  See the License for the specific language governing permissions and\n")
        file.write(" *  limitations under the License.\n")
        file.write(" */\n\n")

        file.write("/*\n")
        file.write(" * This file is auto generated.\n")
        file.write(" */\n\n")

        file.write(f"#ifndef RUNEOS_{font_name_cppified.upper()}_H \n")
        file.write(f"#define RUNEOS_{font_name_cppified.upper()}_H\n\n")

        file.write("#include <KernelRuntime/FrameBuffer.h>\n\n")

        file.write("namespace Rune {\n")

        # Glyphs to byte array
        glyph_idx = font.number_of_glyphs * font.glyph_size
        file.write(f"    static uint8_t {font_name_cppified}_glyphs[{glyph_idx}] = {'{'}\n")

        byte_width = int(font.pixel_width / 8)
        if font.pixel_width % 8 != 0:
            byte_width += 1

        glyph_id = 0
        for g in font.glyphs:
            file.write(f"            // Glyph {glyph_id}\n")
            for i in range(0, font.glyph_size, byte_width):
                vi = 0
                # write byte values
                while vi < byte_width:
                    if vi == 0:
                        file.write("            ")
                    file.write(f"{format(g[i + vi], '#04x')}, ")
                    vi += 1

                ci = 0
                # write comment
                while ci < byte_width:
                    if ci == 0:
                        file.write("        // ")
                    file.write(f"{format(g[i + ci], '08b')}")
                    ci += 1
                file.write("\n")
            file.write("\n")
            glyph_id += 1
        file.write("    };\n\n")

        # Font object definition
        file.write(f"    static BitMapFont {font_name_cppified} = {'{'}\n")
        file.write(f'            "{font.name}",\n')
        file.write(f"            {font.number_of_glyphs},\n")
        file.write(f"            {font.glyph_size},\n")
        file.write(f"            {font.pixel_height},\n")
        file.write(f"            {font.pixel_width},\n")
        file.write(f"            {font_name_cppified}_glyphs\n")
        file.write("    };\n")

        file.write("}\n\n")
        file.write(f"#endif //RUNEOS_{font_name_cppified.upper()}_H")


@click.command()
@click.argument("out_file", type=Path)
@click.argument("font_file", type=Path)
def convert_psf2_to_cpp(font_file: Path, out_file: Path) -> None:
    """Read the given PSF2 font file and try to generate a CPP header for the bitmap font.

    :param font_file: PSF2 font file.
    :param out_file:  Output file for the header file
    :return: True if the header file has been generated else false.
    """
    psf_font = read_psf2_file(font_file)
    if not psf_font:
        return
    generate_cpp_font_file(out_file, psf_font)


if __name__ == "__main__":
    convert_psf2_to_cpp()
