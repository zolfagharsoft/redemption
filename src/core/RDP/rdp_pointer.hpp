/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "gdi/screen_info.hpp"
#include "utils/bitfu.hpp"
#include "utils/sugar/bytes_view.hpp"

class InStream;
class OutStream;

struct CursorSize
{
    uint16_t width;
    uint16_t height;

    constexpr explicit CursorSize(uint16_t width, uint16_t height) noexcept
    : width(width)
    , height(height)
    {}
};

struct Hotspot
{
    uint16_t x;
    uint16_t y;

    constexpr explicit Hotspot(uint16_t x, uint16_t y) noexcept
    : x(x)
    , y(y)
    {}
};

struct RdpPointerView
{
    RdpPointerView() = default;

    constexpr explicit RdpPointerView(
        CursorSize dimensions,
        Hotspot hotspot,
        BitsPerPixel xor_bits_per_pixel,
        bytes_view xor_mask,
        bytes_view and_mask
    ) noexcept
    : dimensions_(dimensions)
    , hotspot_(hotspot)
    , xor_bits_per_pixel_(xor_bits_per_pixel)
    , xor_mask_(xor_mask)
    , and_mask_(and_mask)
    {
        assert(xor_bits_per_pixel == BitsPerPixel(0)
            || compute_mask_line_size(dimensions.width, xor_bits_per_pixel) != 0);
        assert(xor_mask.size() == dimensions.height
                                * compute_mask_line_size(dimensions.width, xor_bits_per_pixel));
        assert(and_mask.size() == dimensions.height
                                * compute_mask_line_size(dimensions.width, BitsPerPixel(1)));
    }

    constexpr CursorSize dimensions() const noexcept
    {
        return this->dimensions_;
    }

    constexpr Hotspot hotspot() const noexcept
    {
        return this->hotspot_;
    }

    constexpr BitsPerPixel xor_bits_per_pixel() const noexcept
    {
        return this->xor_bits_per_pixel_;
    }

    // padded to a 2-byte boundary
    constexpr bytes_view xor_mask() const noexcept
    {
        return this->xor_mask_;
    }

    // padded to a 2-byte boundary
    constexpr bytes_view and_mask() const noexcept
    {
        return this->and_mask_;
    }

    // padded to a 2-byte boundary
    constexpr static uint32_t compute_mask_line_size(
        uint16_t width, BitsPerPixel bits_per_pixel) noexcept
    {
        switch (bits_per_pixel) {
            case BitsPerPixel::BitsPP1:
                return ::even_pad_length(::nbbytes(width));

            case BitsPerPixel::BitsPP4:
                return ::even_pad_length(::nbbytes(width * 4));

            case BitsPerPixel::BitsPP8:
                return ::even_pad_length(width);

            case BitsPerPixel::BitsPP15:
            case BitsPerPixel::BitsPP16:
                return width * 2;

            case BitsPerPixel::BitsPP24:
                return ::even_pad_length(width * 3);

            case BitsPerPixel::BitsPP32:
                return width * 4;

            case BitsPerPixel::Unspecified:
                break;
        }

        return 0;
    }

private:
    CursorSize dimensions_ {0, 0};
    Hotspot hotspot_ {0, 0};
    BitsPerPixel xor_bits_per_pixel_;
    bytes_view xor_mask_;
    bytes_view and_mask_;
};


struct Pointer
{
    enum  {
        POINTER_NULL             ,
        POINTER_NORMAL           ,
        POINTER_EDIT             ,
        POINTER_DRAWABLE_DEFAULT ,
        POINTER_SYSTEM_DEFAULT   ,
        POINTER_SIZENESW         ,  // Double-pointed arrow pointing northeast and southwest
        POINTER_SIZENS           ,  // Double-pointed arrow pointing north and south
        POINTER_SIZENWSE         ,  // Double-pointed arrow pointing northwest and southeast
        POINTER_SIZEWE           ,  // Double-pointed arrow pointing west and east
        POINTER_DOT              ,  // Little Dot of 5x5 pixels

        POINTER_CUSTOM
    };

public:
    // Bitmap sizes (in bytes)
    enum {
          MAX_WIDTH  = 96
        , MAX_HEIGHT = 96
        , MAX_BPP    = 32
    };
    enum {
          DATA_SIZE = MAX_WIDTH * MAX_HEIGHT * MAX_BPP / 8
        , MASK_SIZE = MAX_WIDTH * MAX_HEIGHT * 1 / 8
    };

private:
    // TODO OPTIMIZATION initialize with respect to dimensions
    uint8_t data[DATA_SIZE] {};
    uint8_t mask[MASK_SIZE] {};

    CursorSize dimensions {32,32};
    Hotspot hotspot {0, 0};

    BitsPerPixel native_xor_bpp { BitsPerPixel{0} };

    uint16_t native_length_and_mask { 0 };
    uint16_t native_length_xor_mask { 0 };

public:
    constexpr explicit Pointer() = default;

    explicit Pointer(RdpPointerView pointer)
    {
        *this = build_from_native(
            pointer.dimensions(), pointer.hotspot(),
            pointer.xor_bits_per_pixel(), pointer.xor_mask(), pointer.and_mask());
    }

    Pointer& operator=(RdpPointerView pointer)
    {
        *this = Pointer(pointer);
        return *this;
    }

    operator RdpPointerView () const
    {
        return RdpPointerView(
            get_dimensions(), get_hotspot(), get_native_xor_bpp(),
            get_native_xor_mask(), get_monochrome_and_mask());
    }

    template<class Builder>
    constexpr static Pointer build_from(CursorSize d, Hotspot hs, BitsPerPixel bits_per_pixel, Builder&& builder)
    {
        Pointer pointer;

        pointer.dimensions = d;
        pointer.hotspot = hs;
        pointer.native_xor_bpp = bits_per_pixel;
        pointer.native_length_xor_mask
          = checked_int(d.height * even_pad_length(d.width * nbbytes(underlying_cast(bits_per_pixel))));
        pointer.native_length_and_mask
          = checked_int(d.height * even_pad_length(nbbytes(d.width)));

        builder(pointer.data, pointer.mask);

        return pointer;
    }

    static Pointer build_from_native(CursorSize d, Hotspot hs, BitsPerPixel xor_bpp, bytes_view xor_mask, bytes_view and_mask);

    bool operator==(const Pointer & other) const;

    [[nodiscard]] CursorSize get_dimensions() const
    {
        return this->dimensions;
    }

    [[nodiscard]] Hotspot get_hotspot() const
    {
        return this->hotspot;
    }

    // size is a multiple of 2
    [[nodiscard]] bytes_view get_monochrome_and_mask() const
    {
        return {this->mask, this->bit_mask_size()};
    }

    [[nodiscard]] unsigned bit_mask_size() const
    {
        return this->dimensions.height * ::even_pad_length(::nbbytes(this->dimensions.width));
    }

    [[nodiscard]] bool is_valid() const
    {
        return (this->dimensions.width != 0 && this->dimensions.height != 0/* && this->bpp*/);
    }

    [[nodiscard]] BitsPerPixel get_native_xor_bpp() const { return native_xor_bpp; }

    [[nodiscard]] bytes_view get_native_xor_mask() const
    {
        return {this->data, this->native_length_xor_mask};
    }
};


//    2.2.9.1.1.4.4     Color Pointer Update (TS_COLORPOINTERATTRIBUTE)
//    -----------------------------------------------------------------
//    The TS_COLORPOINTERATTRIBUTE structure represents a regular T.128 24 bpp
//    color pointer, as specified in [T128] section 8.14.3. This pointer update
//    is used for both monochrome and color pointers in RDP.

//    2.2.9.1.1.4.5    New Pointer Update (TS_POINTERATTRIBUTE)
//    ---------------------------------------------------------
//    The TS_POINTERATTRIBUTE structure is used to send pointer data at an
//    arbitrary color depth. Support for the New Pointer Update is advertised
//    in the Pointer Capability Set (section 2.2.7.1.5).

bool emit_native_pointer(OutStream& stream, uint16_t cache_idx, RdpPointerView const& cursor);


RdpPointerView pointer_loader_new(BitsPerPixel data_bpp, InStream& stream);

RdpPointerView pointer_loader_2(InStream & stream);

RdpPointerView pointer_loader_32x32(InStream & stream);

Pointer const& normal_pointer() noexcept;
Pointer const& edit_pointer() noexcept;
Pointer const& drawable_default_pointer() noexcept;
Pointer const& size_NS_pointer() noexcept;
Pointer const& size_NESW_pointer() noexcept;
Pointer const& size_NWSE_pointer() noexcept;
Pointer const& size_WE_pointer() noexcept;
Pointer const& dot_pointer() noexcept;
Pointer const& null_pointer() noexcept;
Pointer const& system_normal_pointer() noexcept;
Pointer const& system_default_pointer() noexcept;
