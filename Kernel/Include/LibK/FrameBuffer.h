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

#ifndef RUNEOS_FRAMEBUFFER_H
#define RUNEOS_FRAMEBUFFER_H


#include <Hammer/Definitions.h>


namespace Rune::LibK {

    /**
     * @brief An in memory declaration of a bitmap font, glyphs are encoded in a byte array.
     */
    struct BitMapFont {
        const char* name;
        U32 number_of_glyphs;
        U32 glyph_size;
        U32 pixel_height;
        U32 pixel_width;
        U8* glyphs;
    };


    /**
     * @brief An RGBA pixel.
     */
    struct Pixel {
        U8 red   = 0;
        U8 green = 0;
        U8 blue  = 0;
        U8 alpha = 0;


        bool operator==(const Pixel& o) const;


        bool operator!=(const Pixel& o) const;
    };


    /**
     * @brief 2D position in the frame buffer.
     */
    struct Coord2D {
        U32 x;
        U32 y;
    };


    /**
     * @brief Common pixel colors.
     */
    namespace Pixie {
        constexpr Pixel BLACK = { 0, 0, 0, 0 };
        constexpr Pixel WHITE = { 255, 255, 255, 0 };
        constexpr Pixel RED   = { 255, 0, 0, 0 };
        constexpr Pixel GREEN = { 0, 255, 0, 0 };
        constexpr Pixel BLUE  = { 0, 0, 255, 0 };


        constexpr Pixel VSCODE_CYAN   = { 17, 168, 205, 0 };
        constexpr Pixel VSCODE_BLUE   = { 36, 114, 200, 0 };
        constexpr Pixel VSCODE_YELLOW = { 229, 229, 16, 0 };
        constexpr Pixel VSCODE_WHITE  = { 229, 229, 229, 0 };
        constexpr Pixel VSCODE_RED    = { 205, 49, 49, 0 };
    }


    /**
     * @brief A frame buffer implementation which provides basic glyph rendering.
     */
    class FrameBuffer {
        static constexpr U8 BITS_PER_TYPE = 8;

        U8* _address = nullptr;
        U64 _width           = 0;
        U64 _height          = 0;
        U64 _pitch           = 0;
        U16 _bpp             = 0; // bits per pixel
        U8  _red_shift       = 0;
        U8  _green_shift     = 0;
        U8  _blue_shift      = 0;
        U8  _bytes_per_pixel = 0;

        static FrameBuffer _instance;

        void draw_perpendicular(
                int x0,
                int y0,
                int threshold,
                int e_diag,
                int e_square,
                int dx,
                int dy,
                int e_init,
                double width,
                int w_init,
                bool sy,
                U8* raw_pixel
                ) const;


    public:
        FrameBuffer();


        FrameBuffer(
                U8* address,
                U64 width,
                U64 height,
                U64 pitch,
                U16 bpp,
                U8 red_shift,
                U8 green_shift,
                U8 blue_shift
        );


        /**
         * Set the framebuffer for global access by other kernel modules. 
         *
         * @param frame_buffer Bootloader provided framebuffer.
         */
        static void set_global(FrameBuffer frame_buffer);


        /**
         * 
         * @return The boot loader provided framebuffer.
         */
        static FrameBuffer* get_global();


        /**
         * 
         * @return Pointer to the framebuffer.
         */
        [[nodiscard]] U8* get_address() const;


        /**
         * 
         * @return Number of pixels in a line.
         */
        [[nodiscard]] U64 get_width() const;


        /**
         * 
         * @return Number of pixels in a column.
         */
        [[nodiscard]] U64 get_height() const;


        /**
         * 
         * @return Number of bytes in a line.
         */
        [[nodiscard]] U64 get_pitch() const;


        /**
         * 
         * @return Number of bits in a pixel.
         */
        [[nodiscard]] U16 get_bits_per_pixel() const;


        /**
         * Convert the pixel to it's physical layout in memory, e.g. rgb or rbg.
         *
         * @param pixel Virtual pixel.
         * @param raw_pixel_out Output buffer for the physical pixel.
         */
        void to_raw_pixel(const Pixel& pixel, U8 raw_pixel_out[4]) const;


        /**
         * Draw the bitmap font glyph of a character.
         *
         * @param font Bitmap font.
         * @param x X-coordinate of the upper left corner of the glyph.
         * @param y Y-coordinate of the upper left corner of the glyph.
         * @param bg_color Background color.
         * @param fg_color Foreground color.
         * @param ch ASCII code.
         */
        void draw_glyph(BitMapFont* font, U32 x, U32 y, Pixel bg_color, Pixel fg_color, char ch) const;


        void draw_line(Coord2D start, Coord2D end, Pixel color, double thickness) const;
    };
}

#endif //RUNEOS_FRAMEBUFFER_H
