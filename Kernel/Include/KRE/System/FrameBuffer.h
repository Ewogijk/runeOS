
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

#include <Ember/Ember.h>

#include <KRE/Collections/Array.h>
#include <KRE/Utility.h>

namespace Rune {
    /**
     * @brief An in memory declaration of a bitmap font, glyphs are encoded in a byte array.
     */
    struct BitMapFont {
        const char* name;
        U32         number_of_glyphs;
        U32         glyph_size;
        U32         pixel_height;
        U32         pixel_width;
        U8*         glyphs;
    };

    /**
     * @brief 2D position in the frame buffer.
     */
    struct Coord2D {
        U32 x;
        U32 y;
    };

    /**
     * @brief A frame buffer implementation which provides basic glyph rendering.
     */
    class FrameBuffer {
        static constexpr U8 BITS_PER_BYTE = 8;
        static constexpr U8 BYTE_MASK     = 0xFF;
        static constexpr U8 RGBA_OFFSET_R = 0;
        static constexpr U8 RGBA_OFFSET_G = 8;
        static constexpr U8 RGBA_OFFSET_B = 16;
        static constexpr U8 RGBA_OFFSET_A = 24;

        U8* _address         = nullptr;
        U64 _width           = 0;
        U64 _height          = 0;
        U64 _pitch           = 0;
        U16 _bpp             = 0; // bits per pixel
        U8  _red_shift       = 0;
        U8  _green_shift     = 0;
        U8  _blue_shift      = 0;
        U8  _bytes_per_pixel = 0;

        static FrameBuffer _instance;

        void draw_perpendicular(int    x0,
                                int    y0,
                                int    threshold,
                                int    e_diag,
                                int    e_square,
                                int    dx,
                                int    dy,
                                int    e_init,
                                double width,
                                int    w_init,
                                bool   sy,
                                U8*    raw_pixel) const;

      public:
        FrameBuffer();

        FrameBuffer(U8* address,
                    U64 width,
                    U64 height,
                    U64 pitch,
                    U16 bpp,
                    U8  red_shift,
                    U8  green_shift,
                    U8  blue_shift);

        /**
         * Set the framebuffer for global access by other kernel modules.
         *
         * @param frame_buffer Bootloader provided framebuffer.
         */
        static void set_global(const FrameBuffer& frame_buffer);

        /**
         *
         * @return The bootloader provided framebuffer.
         */
        static auto get_global() -> FrameBuffer*;

        /**
         *
         * @return Pointer to the framebuffer.
         */
        [[nodiscard]] auto get_address() const -> U8*;

        /**
         *
         * @return Number of pixels in a line.
         */
        [[nodiscard]] auto get_width() const -> U64;

        /**
         *
         * @return Number of pixels in a column.
         */
        [[nodiscard]] auto get_height() const -> U64;

        /**
         *
         * @return Number of bytes in a line.
         */
        [[nodiscard]] auto get_pitch() const -> U64;

        /**
         *
         * @return Number of bits in a pixel.
         */
        [[nodiscard]] auto get_bits_per_pixel() const -> U16;

        /**
         * Convert the pixel to it's physical layout in memory, e.g. rgb or rbg.
         *
         * @param pixel Virtual pixel.
         * @param raw_pixel_out Output buffer for the physical pixel.
         */
        void to_raw_pixel(const Pixel& pixel, Array<U8, 4>& raw_pixel_out) const;

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
        void draw_glyph(const BitMapFont* font,
                        U32               x,
                        U32               y,
                        Pixel             bg_color,
                        Pixel             fg_color,
                        char              ch) const;

        void draw_line(Coord2D start, Coord2D end, Pixel color, double thickness) const;
    };
} // namespace Rune

#endif // RUNEOS_FRAMEBUFFER_H
