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
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include <vector>
#include <string>
#include <cstdint>


namespace rdpdr
{
    // forward declaration from core/RDP/channels/rdpdr.hpp
    enum RDPDR_DTYP : uint32_t;
}

struct RDPDiskConfig
{
    struct DeviceInfo {
        char name[8];
        rdpdr::RDPDR_DTYP type;

        DeviceInfo(const char * name, rdpdr::RDPDR_DTYP type) noexcept;
    };

    std::vector<DeviceInfo> device_list;

    RDPDiskConfig() noexcept;

    void add_drive(const std::string & name, rdpdr::RDPDR_DTYP type);

    bool enable_drive_type = true;
    bool enable_printer_type = true;
    bool enable_port_type = true;
    bool enable_smart_card_type = true;

    uint32_t ioCode1;
    uint32_t extendedPDU;
    uint32_t extraFlags1;
    uint32_t SpecialTypeDeviceCap;
    uint32_t general_capability_version;
};

