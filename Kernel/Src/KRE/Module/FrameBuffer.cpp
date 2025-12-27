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

#include <KRE/System/FrameBuffer.h>

#include <limits.h> // NOLINT climits does not exist

#include <KRE/CppRuntimeSupport.h>
#include <KRE/Math.h>
#include <KRE/Utility.h>

namespace Rune {
    auto Pixel::operator==(const Pixel& o) const -> bool {
        return red == o.red && green == o.green && blue == o.blue && alpha == o.alpha;
    }

    auto Pixel::operator!=(const Pixel& o) const -> bool {
        return red != o.red || green != o.green || blue != o.blue || alpha != o.alpha;
    }

    FrameBuffer FrameBuffer::_instance = FrameBuffer();

    FrameBuffer::FrameBuffer() = default;

    FrameBuffer::FrameBuffer(U8*       address,
                             const U64 width,
                             const U64 height,
                             const U64 pitch,
                             const U16 bpp,
                             const U8  red_shift,
                             const U8  green_shift,
                             const U8  blue_shift)
        : _address(address),
          _width(width),
          _height(height),
          _pitch(pitch),
          _bpp(bpp),
          _red_shift(red_shift),
          _green_shift(green_shift),
          _blue_shift(blue_shift),
          _bytes_per_pixel(_bpp / BITS_PER_BYTE) {}

    void FrameBuffer::set_global(const FrameBuffer& frame_buffer) {
        if (_instance._address == nullptr) {
            _instance = frame_buffer;
        }
    }

    auto FrameBuffer::get_global() -> FrameBuffer* {
        return _instance._address == nullptr ? nullptr : &_instance;
    }

    auto FrameBuffer::get_address() const -> U8* { return _address; }

    auto FrameBuffer::get_width() const -> U64 { return _width; }

    auto FrameBuffer::get_height() const -> U64 { return _height; }

    auto FrameBuffer::get_pitch() const -> U64 { return _pitch; }

    auto FrameBuffer::get_bits_per_pixel() const -> U16 { return _bpp; }

    void FrameBuffer::to_raw_pixel(const Pixel& pixel, Array<U8, 4>& raw_pixel_out) const {
        U32 temp_px  = 0;
        temp_px     |= (pixel.red << _red_shift);
        temp_px     |= (pixel.green << _green_shift);
        temp_px     |= (pixel.blue << _blue_shift);

        constexpr U8 rgba_px_size = 32;
        if (_bpp == rgba_px_size) temp_px |= (pixel.alpha << RGBA_OFFSET_A);
        raw_pixel_out[0] = temp_px & BYTE_MASK;
        raw_pixel_out[1] = (temp_px >> RGBA_OFFSET_G) & BYTE_MASK;
        raw_pixel_out[2] = (temp_px >> RGBA_OFFSET_B) & BYTE_MASK;
        raw_pixel_out[3] = (temp_px >> RGBA_OFFSET_A) & BYTE_MASK;
    }

    void FrameBuffer::draw_glyph(const BitMapFont* font,
                                 const U32         x,
                                 const U32         y,
                                 const Pixel       bg_color,
                                 const Pixel       fg_color,
                                 const char        ch) const {
        // Look up the bitmap of the glyph to render
        U8 glyph[font->glyph_size]; // NOLINT is dynamic size so keep as is
        for (U32 i = 0; i < font->glyph_size; i++) {
            glyph[i] = font->glyphs[(static_cast<U8>(ch) * font->glyph_size) + i];
        }
        U8 row_width = font->pixel_width / BITS_PER_BYTE;
        if (font->pixel_width - row_width * BITS_PER_BYTE > 0) row_width++;

        Array<U8, 4> bg_c{};
        to_raw_pixel(bg_color, bg_c);
        Array<U8, 4> fg_c{};
        to_raw_pixel(fg_color, fg_c);

        const U64 fb_x_reset = x * static_cast<U64>(_bytes_per_pixel);
        U64       fb_x_off   = fb_x_reset;
        U64       fb_y_off   = (y * _pitch);

        for (U64 fy = 0; fy < font->glyph_size; fy += row_width) {
            U8 row_pos = fy;
            for (U64 fx = 0; fx < font->pixel_width; fx++) {
                if (fx > 0 && fx % BITS_PER_BYTE == 0) row_pos++;
                if (((glyph[row_pos] >> (BITS_PER_BYTE - 1 - fx)) & 0x1) == 1)
                    memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(_address) + fb_y_off
                                                   + fb_x_off),
                           fg_c.data(),
                           4);
                else
                    memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(_address) + fb_y_off
                                                   + fb_x_off),
                           bg_c.data(),
                           4);
                fb_x_off += _bytes_per_pixel;
            }

            fb_x_off  = fb_x_reset; // New line -> Reset x offset
            fb_y_off += _pitch;
        }
    }

    auto sqrt(const double num) -> double {
        if (num < 0) return -1.0;
        constexpr U8 precision = 20;
        double       left      = 0;
        double       right     = num + 1;
        for (U8 i = 0; i < precision; i++) {
            if (const double middle = (left + right) / 2; middle * middle < num) {
                left = middle;
            } else {
                right = middle;
            }
        }
        return left;
    }

    auto FrameBuffer::draw_perpendicular(int    x0,
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
                                         U8*    raw_pixel) const -> void {
        // These values somehow define the width of the perpendicular line, but I have no clue
        // what's going on
        const double w_threshold = 2 * width * sqrt((dx * dx) + (dy * dy));
        int          tk          = dx + dy - w_init;
        int          x           = x0;
        int          y           = y0;
        int          error       = e_init;

        // Draw the perpendicular to the left up/down
        while (tk <= w_threshold) {
            memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(_address) + y + x),
                   raw_pixel,
                   4);
            if (error > threshold) {
                x     -= _bytes_per_pixel;
                error += e_diag;
                tk     = tk + 2 * dy;
            }
            error += e_square;
            y      = sy ? y + static_cast<int>(_pitch) : y - static_cast<int>(_pitch);
            tk     = tk + 2 * dx;
        }

        x     = x0;
        y     = y0;
        error = -e_init;
        tk    = dx + dy + w_init;

        // Draw the perpendicular to the right up/down
        while (tk <= w_threshold) {
            memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(_address) + y + x),
                   raw_pixel,
                   4);
            if (error > threshold) {
                x     += _bytes_per_pixel;
                error += e_diag;
                tk    += 2 * dy;
            }
            error += e_square;
            y      = !sy ? y + static_cast<int>(_pitch) : y - static_cast<int>(_pitch);
            tk    += 2 * dx;
        }
    }

    void FrameBuffer::draw_line(Coord2D      start, // NOLINT it works do not refactor
                                Coord2D      end,
                                const Pixel  color,
                                const double thickness) const {
        if (thickness <= 0)
            // Negative thickness is garbage and if thickness=0 -> Line is invisible
            return;
        // Bresenham's algorithm
        Array<U8, 4> raw_color{};
        to_raw_pixel(color, raw_color);

        // General note on x and y coords
        // They are given in terms of pixel location, but we need them in terms of offset into the
        // linear framebuffer. Conversions are: x_fb = x_px * _bytes_per_pixel, y_fb = y_px * _pitch
        if (start.x == end.x) {
            // Vertical line -> Line equation is undefined (division by zero), but can easily
            // iterate y-axis
            if (start.y > end.y) swap(start, end);

            // We want to draw (thickness / 2) lines to the left and right of the center line, the
            // line directly at y position. Visualized the algorithm draws this:
            //  thickness=1     thickness=2     thickness=3
            //      c                c              c
            //      |               ||             |||
            //      |               ||             |||
            // For even thickness we draw one line more to the left
            const U32 f_t_half = static_cast<U32>(thickness / 2);
            const U32 x_start = (start.x >= f_t_half ? (start.x - f_t_half) : 0) * _bytes_per_pixel;
            const U32 x_end   = (start.x + static_cast<U32>(thickness / 2) + 1) * _bytes_per_pixel;

            for (U32 x = x_start; x < x_end; x += _bytes_per_pixel)
                for (U32 y = start.y * _pitch; y < end.y * _pitch; y += _pitch)
                    memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(_address) + y + x),
                           raw_color.data(),
                           _bytes_per_pixel);
        } else if (start.y == end.y) {
            // Horizontal line -> Can simplify the loop
            if (start.x > end.x) swap(start, end);

            // Same principle as in the vertical line case: Draw (thickness / 2) lines above and
            // beneath the center line Visualized: thickness=1  c ---
            //
            //                ---
            // thickness=2  c ---
            //
            //                ---
            // thickness=3  c ---
            //                ---
            const U32 f_t_half = static_cast<U32>(thickness / 2);
            const U32 y_start  = (start.y >= f_t_half ? (start.y - f_t_half) : 0) * _pitch;
            const U32 y_end    = (start.y + static_cast<U32>(thickness / 2) + 1) * _pitch;
            for (U32 y = y_start; y < y_end; y += _pitch)
                for (U32 x  = start.x * _bytes_per_pixel; x < end.x * _bytes_per_pixel;
                     x     += _bytes_per_pixel)
                    memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(_address) + y + x),
                           raw_color.data(),
                           _bytes_per_pixel);
        } else {
            // Draw line in any other direction using the line equation
            // Basic Bresenham allows us to only draw in quadrant 0 in increasing order of y (y
            // grows down) and x coords
            //                y
            //              2 | 1
            //             -------x
            //              3 | 0
            //
            if (start.x > end.x)
                // End point is to the left of the start, we would draw in decreasing order, but it
                // does not matter if we draw from right to left or left to right -> Swap! So we
                // have easier math, this means essentially we allow drawing in quadrant 3
                swap(start, end);
            const int dx = static_cast<int>(end.x) - static_cast<int>(start.x);
            int       dy{0};
            bool      sy{false};
            // We keep track of the sign of dy, this allows to not only draw from top to bottom but
            // also the other way end.y > start.y -> increment y, end.y < start.y -> decrement y
            // Swapping will mess things up, so we have to live with harder math, the bright side is
            // we can draw in quadrant 1 and 3 now!
            if (end.y > start.y) {
                dy = static_cast<int>(end.y) - static_cast<int>(start.y);
                sy = true;
            } else {
                dy = static_cast<int>(start.y) - static_cast<int>(end.y);
                sy = false;
            }
            // We assume the pixel is in the center of a square
            // |---|
            // |   |
            // | x<|-- Pixel
            // |   |
            // |---|
            // The problem is that the line we will calculate will not always go through the center
            // of the pixels, if error=0.5 than we intersect the pixel center, error=0.9 means the
            // line goes above the pixel center We can utilize this to time the y increments, since
            // if error>=1.0 the line is on the next row in the display
            int p_error = 0; // Error for the perpendiculars
            int error   = 0; // Error for the center line
            int y       = static_cast<int>(start.y) * static_cast<int>(_pitch);
            int x       = static_cast<int>(start.x) * static_cast<int>(_bytes_per_pixel);
            // We essentially solve the line equation y=mx+b -> we could increment the error by m
            // and be done, but this is slower because it uses floating point arithmetic Using this
            // threshold, e_diag and e_square values we make it integer arithmetic only which is
            // better but more complex and I don't know what's going on
            const int threshold = dx - (2 * dy);
            const int e_diag    = -2 * dx;
            const int e_square  = 2 * dy;
            for (int i = 0; i < dx; i++) {
                memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(_address) + y + x),
                       (void*) raw_color.data(),
                       4);
                // To make diagonal lines thicker we will draw perpendicular lines to the left and
                // right of the center line, this will be done whenever a pixel of the center line
                // is drawn:
                //
                //    kinda perpendicular line
                //        x
                //         x   xx
                //         x xx
                //   center x
                //           x
                //           x
                //      kinda perpendicular line
                draw_perpendicular(x,
                                   y,
                                   dx,
                                   dy,
                                   threshold,
                                   e_diag,
                                   e_square,
                                   p_error,
                                   thickness,
                                   error,
                                   sy,
                                   raw_color.data());
                if (error > threshold) {
                    y      = sy ? y + static_cast<int>(_pitch) : y - static_cast<int>(_pitch);
                    error += e_diag;
                    if (p_error > threshold) {
                        // When we do not draw the additional perpendicular line, the final line
                        // will have gaps in between the perpendicular lines, because we only draw
                        // lines perpendicular to the point when we advance in x direction, but we
                        // also need lines perpendicular to the point when we advance in y direction
                        //
                        //        x x
                        //         x x xx
                        //         x xx
                        //   center x
                        //           x
                        //           x
                        draw_perpendicular(x,
                                           y,
                                           dx,
                                           dy,
                                           threshold,
                                           e_diag,
                                           e_square,
                                           p_error + e_diag + e_square,
                                           thickness,
                                           error,
                                           sy,
                                           raw_color.data());
                        p_error += e_diag;
                    }
                    p_error += e_square;
                }
                error += e_square;
                x     += _bytes_per_pixel;
            }
        }
    }
} // namespace Rune
