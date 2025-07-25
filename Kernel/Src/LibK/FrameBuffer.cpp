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

#include <LibK/FrameBuffer.h>

#include <Hammer/Memory.h>
#include <Hammer/Utility.h>
#include <Hammer/Math.h>


namespace Rune::LibK {

    bool Pixel::operator==(const Pixel& o) const {
        return red == o.red && green == o.green && blue == o.blue && alpha == o.alpha;
    }


    bool Pixel::operator!=(const Pixel& o) const {
        return !(red == o.red && green == o.green && blue == o.blue && alpha == o.alpha);
    }


    FrameBuffer FrameBuffer::_instance = FrameBuffer();


    FrameBuffer::FrameBuffer() : _address(nullptr),
                                 _width(0),
                                 _height(0),
                                 _pitch(0),
                                 _bpp(0),
                                 _red_shift(0),
                                 _green_shift(0),
                                 _blue_shift(0) {

    }


    FrameBuffer::FrameBuffer(
            U8* address,
            U64 width,
            U64 height,
            U64 pitch,
            U16 bpp,
            U8 red_shift,
            U8 green_shift,
            U8 blue_shift
    ) : _address(address),
        _width(width),
        _height(height),
        _pitch(pitch),
        _bpp(bpp),
        _red_shift(red_shift),
        _green_shift(green_shift),
        _blue_shift(blue_shift) {
        _bytes_per_pixel = _bpp / BITS_PER_TYPE;
    }


    void FrameBuffer::set_global(FrameBuffer frame_buffer) {
        if (_instance._address == nullptr) {
            _instance = frame_buffer;
        }
    }


    FrameBuffer* FrameBuffer::get_global() {
        return _instance._address == nullptr ? nullptr : &_instance;
    }


    U8* FrameBuffer::get_address() const {
        return _address;
    }


    U64 FrameBuffer::get_width() const {
        return _width;
    }


    U64 FrameBuffer::get_height() const {
        return _height;
    }


    U64 FrameBuffer::get_pitch() const {
        return _pitch;
    }


    U16 FrameBuffer::get_bits_per_pixel() const {
        return _bpp;
    }


    void FrameBuffer::to_raw_pixel(const Pixel& pixel, U8* raw_pixel_out) const {
        U32 temp_px = 0;
        temp_px |= (pixel.red << _red_shift);
        temp_px |= (pixel.green << _green_shift);
        temp_px |= (pixel.blue << _blue_shift);
        if (_bpp == 32)
            temp_px |= (pixel.alpha << 24);
        raw_pixel_out[0] = temp_px & 0xFF;
        raw_pixel_out[1] = (temp_px >> 8) & 0xFF;
        raw_pixel_out[2] = (temp_px >> 16) & 0xFF;
        raw_pixel_out[3] = (temp_px >> 24) & 0xFF;
    }


    void FrameBuffer::draw_glyph(
            BitMapFont* font,
            U32 x,
            U32 y,
            Pixel bg_color,
            Pixel fg_color,
            char ch
    ) const {
        // Look up the bitmap of the glyph to render
        U8       glyph[font->glyph_size];
        for (U32 i         = 0; i < font->glyph_size; i++) {
            glyph[i] = font->glyphs[((U8) ch * font->glyph_size) + i];
        }
        U8       row_width = font->pixel_width / 8;
        if (font->pixel_width - row_width * 8 > 0)
            row_width++;


        U8 bg_c[4];
        to_raw_pixel(bg_color, bg_c);
        U8 fg_c[4];
        to_raw_pixel(fg_color, fg_c);

        U64 fb_x_reset = x * _bytes_per_pixel;
        U64 fb_x_off   = fb_x_reset;
        U64 fb_y_off   = (y * _pitch);

        for (U64 fy = 0; fy < font->glyph_size; fy += row_width) {
            U8       row_pos = fy;
            for (U64 fx      = 0; fx < font->pixel_width; fx++) {
                if (fx > 0 && fx % BITS_PER_TYPE == 0)
                    row_pos++;
                if (((glyph[row_pos] >> (BITS_PER_TYPE - 1 - fx)) & 0x1) == 1)
                    memcpy((void*) ((uintptr_t) _address + fb_y_off + fb_x_off), (void*) fg_c, 4);
                else
                    memcpy((void*) ((uintptr_t) _address + fb_y_off + fb_x_off), (void*) bg_c, 4);
                fb_x_off += _bytes_per_pixel;
            }

            fb_x_off = fb_x_reset; // New line -> Reset x offset
            fb_y_off += _pitch;
        }
    }


    double sqrt(double num) {
        if (num < 0)
            return -1.0;

        double   left  = 0;
        double   right = num + 1;
        for (int i     = 0; i < 20; i++) {
            double middle = (left + right) / 2;
            if (middle * middle < num) {
                left = middle;
            } else {
                right = middle;
            }
        }
        return left;
    }


    void FrameBuffer::draw_perpendicular(
            int x0,
            int y0,
            int dx,
            int dy,
            int threshold,
            int e_diag,
            int e_square,
            int e_init,
            double width,
            int w_init,
            bool sy,
            U8* raw_pixel
    ) const {
        // These values somehow define the width of the perpendicular line, but I have no clue what's going on
        double w_threshold = 2 * width * sqrt(dx * dx + dy * dy);
        int    tk          = dx + dy - w_init;
        int    x           = x0;
        int    y           = y0;
        int    error       = e_init;

        // Draw the perpendicular to the left up/down
        while (tk <= w_threshold) {
            memcpy((void*) ((uintptr_t) _address + y + x), (void*) raw_pixel, 4);
            if (error > threshold) {
                x -= _bytes_per_pixel;
                error += e_diag;
                tk = tk + 2 * dy;
            }
            error += e_square;
            y  = sy ? y + (int) _pitch : y - (int) _pitch;
            tk = tk + 2 * dx;
        }

        x     = x0;
        y     = y0;
        error = -e_init;
        tk    = dx + dy + w_init;

        // Draw the perpendicular to the right up/down
        while (tk <= w_threshold) {
            memcpy((void*) ((uintptr_t) _address + y + x), (void*) raw_pixel, 4);
            if (error > threshold) {
                x += _bytes_per_pixel;
                error += e_diag;
                tk += 2 * dy;
            }
            error += e_square;
            y = !sy ? y + (int) _pitch : y - (int) _pitch;
            tk += 2 * dx;
        }
    }


    void FrameBuffer::draw_line(Coord2D start, Coord2D end, Pixel color, double thickness) const {
        if (thickness <= 0)
            // Negative thickness is garbage and if thickness=0 -> Line is invisible
            return;
        // Bresenham's algorithm
        U8 raw_color[_bytes_per_pixel];
        to_raw_pixel(color, raw_color);

        // General note on x and y coords
        // They are given in terms of pixel location, but we need them in terms of offset into the linear framebuffer.
        // Conversions are: x_fb = x_px * _bytes_per_pixel, y_fb = y_px * _pitch
        if (start.x == end.x) {
            // Vertical line -> Line equation is undefined (division by zero), but can easily iterate y-axis
            if (start.y > end.y)
                swap(start, end);

            // We want to draw (thickness / 2) lines to the left and right of the center line, the line directly at
            // y position.
            // Visualized the algorithm draws this:
            //  thickness=1     thickness=2     thickness=3
            //      c                c              c
            //      |               ||             |||
            //      |               ||             |||
            // For even thickness we draw one line more to the left
            double f_t_half = floor(thickness / 2);
            U32    x_start  = (start.x >= (U32) f_t_half ? (start.x - (U32) f_t_half) : 0) * _bytes_per_pixel;
            U32    x_end    = (start.x + (U32) ceil(thickness / 2)) * _bytes_per_pixel;

            for (U32 x = x_start; x < x_end; x += _bytes_per_pixel)
                for (U32 y = start.y * _pitch; y < end.y * _pitch; y += _pitch)
                    memcpy((void*) ((uintptr_t) _address + y + x), (void*) raw_color, _bytes_per_pixel);
        } else if (start.y == end.y) {
            // Horizontal line -> Can simplify the loop
            if (start.x > end.x)
                swap(start, end);

            // Same principle as in the vertical line case: Draw (thickness / 2) lines above and beneath the center line
            // Visualized:
            // thickness=1  c ---
            //
            //                ---
            // thickness=2  c ---
            //
            //                ---
            // thickness=3  c ---
            //                ---
            double   f_t_half = floor(thickness / 2);
            U32      y_start  = (start.y >= (U32) f_t_half ? (start.y - (U32) f_t_half) : 0) * _pitch;
            U32      y_end    = (start.y + (U32) ceil(thickness / 2)) * _pitch;
            for (U32 y        = y_start; y < y_end; y += _pitch)
                for (U32 x = start.x * _bytes_per_pixel; x < end.x * _bytes_per_pixel; x += _bytes_per_pixel)
                    memcpy((void*) ((uintptr_t) _address + y + x), (void*) raw_color, _bytes_per_pixel);
        } else {
            // Draw line in any other direction using the line equation
            // Basic Bresenham allows us to only draw in quadrant 0 in increasing order of y (y grows down) and x coords
            //                y
            //              2 | 1
            //             -------x
            //              3 | 0
            //
            if (start.x > end.x)
                // End point is to the left of the start, we would draw in decreasing order, but it does not matter
                // if we draw from right to left or left to right -> Swap! So we have easier math, this means
                // essentially we allow drawing in quadrant 3
                swap(start, end);
            int  dx = (int) end.x - (int) start.x;
            int  dy;
            bool sy;
            // We keep track of the sign of dy, this allows to not only draw from top to bottom but also the other way
            // end.y > start.y -> increment y, end.y < start.y -> decrement y
            // Swapping will mess things up, so we have to live with harder math, the bright side is we can draw in
            // quadrant 1 and 3 now!
            if (end.y > start.y) {
                dy = (int) end.y - (int) start.y;
                sy = true;
            } else {
                dy = (int) start.y - (int) end.y;
                sy = false;
            }
            // We assume the pixel is in the center of a square
            // |---|
            // |   |
            // | x<|-- Pixel
            // |   |
            // |---|
            // The problem is that the line we will calculate will not always go through the center of the pixels,
            // if error=0.5 than we intersect the pixel center, error=0.9 means the line goes above the pixel center
            // We can utilize this to time the y increments, since if error>=1.0 the line is on the next row in the
            // display
            int      p_error   = 0;         // Error for the perpendiculars
            int      error     = 0;         // Error for the center line
            int      y         = (int) start.y * (int) _pitch;
            int      x         = (int) start.x * (int) _bytes_per_pixel;
            // We essentially solve the line equation y=mx+b -> we could increment the error by m and be done, but this
            // is slower because it uses floating point arithmetic
            // Using this threshold, e_diag and e_square values we make it integer arithmetic only which is better but
            // more complex and I don't know what's going on
            int      threshold = dx - 2 * dy;
            int      e_diag    = -2 * dx;
            int      e_square  = 2 * dy;
            for (int i         = 0; i < dx; i++) {
                memcpy((void*) ((uintptr_t) _address + y + x), (void*) raw_color, 4);
                // To make diagonal lines thicker we will draw perpendicular lines to the left and right of the center
                // line, this will be done whenever a pixel of the center line is drawn:
                //
                //    kinda perpendicular line
                //        x
                //         x   xx
                //         x xx
                //   center x
                //           x
                //           x
                //      kinda perpendicular line
                draw_perpendicular(
                        x,
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
                        raw_color
                );
                if (error > threshold) {
                    y = sy ? y + (int) _pitch : y - (int) _pitch;
                    error += e_diag;
                    if (p_error > threshold) {
                        // When we do not draw the additional perpendicular line, the final line will have gaps
                        // in between the perpendicular lines, because we only draw lines perpendicular to the point
                        // when we advance in x direction, but we also need lines perpendicular to the point when we
                        // advance in y direction
                        //
                        //        x x
                        //         x x xx
                        //         x xx
                        //   center x
                        //           x
                        //           x
                        draw_perpendicular(
                                x,
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
                                raw_color
                        );
                        p_error += e_diag;
                    }
                    p_error += e_square;
                }
                error += e_square;
                x += _bytes_per_pixel;
            }
        }
    }
}
