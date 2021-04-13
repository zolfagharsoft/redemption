/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include <array>
#include <algorithm>

#include <cassert>


template<class T, std::size_t Max, class IndexType>
struct CacheMapping
{
    using index_type = IndexType;
    using element_type = T;

    static const index_type orignal_max_entries {Max};

    explicit CacheMapping() = default;

    explicit CacheMapping(IndexType max_entries)
    : max_entries(max_entries)
    {
        assert(max_entries <= orignal_max_entries);
        dst_to_src.fill(not_fount);
        src_to_dst.fill(not_fount);
    }

    template<class U>
    void insert(IndexType source_idx, U&& value)
    {
        elements[source_idx] = std::forward<U>(value);
    }

    element_type const& operator[](IndexType i) const noexcept
    {
        return elements[i];
    }

    struct [[nodiscard]] CacheResult
    {
        IndexType destination_idx;
        bool is_cached;
    };

    CacheResult use(IndexType source_idx) noexcept
    {
        ++stamp;

        if (src_to_dst[source_idx] != not_fount) {
            auto idx = src_to_dst[source_idx];
            stamps[idx] = stamp;
            return CacheResult{idx, true};
        }

        IndexType idx;

        if (entries < max_entries) {
            idx = entries;
            ++entries;
        }
        else {
            assert(max_entries > 0);
            // look for oldest
            auto first = stamps.begin();
            auto it = std::min_element(first, first + entries);
            idx = checked_int(std::distance(first, it));

            if (dst_to_src[idx] != not_fount) {
                src_to_dst[dst_to_src[idx]] = not_fount;
            }
        }

        src_to_dst[source_idx] = idx;
        dst_to_src[idx] = source_idx;
        stamps[idx] = stamp;
        return CacheResult{idx, false};
    }

private:
    static constexpr IndexType not_fount = IndexType(~IndexType{});

    std::array<IndexType, Max> src_to_dst;
    std::array<IndexType, Max> dst_to_src;
    std::array<unsigned, Max> stamps {};
    IndexType max_entries {};
    IndexType entries {};
    unsigned stamp {};
    std::array<T, Max> elements;
};
