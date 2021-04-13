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

#include "utils/sugar/noncopyable.hpp"
#include "utils/cache_mapping.hpp"
#include "core/error.hpp"
#include "cxx/cxx.hpp"

#include "core/RDP/rdp_pointer.hpp"


/* difference caches */
class PointerCache : noncopyable
{
public:
    static constexpr std::size_t MAX_POINTER_COUNT = 25;

private:
    using Cache = CacheMapping<Pointer, MAX_POINTER_COUNT, uint16_t>;

public:
    using CacheResult = Cache::CacheResult;

    explicit PointerCache() = default;

    explicit PointerCache(uint16_t pointer_cache_entries)
    : cache(pointer_cache_entries)
    {}

    static uint16_t max_size() noexcept
    {
        return MAX_POINTER_COUNT;
    }

    void insert(uint16_t cache_idx, RdpPointerView const& cursor)
    {
        check(cache_idx);
        cache.insert(cache_idx, cursor);
    }

    CacheResult use(uint16_t cache_idx) noexcept
    {
        check(cache_idx);
        return cache.use(cache_idx);
    }

    Pointer const& pointer(uint16_t cache_idx) const noexcept
    {
        return cache[cache_idx];
    }

private:
    inline static void check(uint16_t cache_idx)
    {
        if (REDEMPTION_UNLIKELY(cache_idx >= max_size())) {
            LOG(LOG_ERR,
                "PointerCache::insert pointer cache idx overflow (%u)",
                cache_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }
    }

    Cache cache;
};  // struct PointerCache
