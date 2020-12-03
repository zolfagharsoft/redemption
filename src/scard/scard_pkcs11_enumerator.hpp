/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2020
    Author(s): 
*/

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "scard/scard_pkcs11_common.hpp"
#include "scard/scard_x509.hpp"


///////////////////////////////////////////////////////////////////////////////


class scard_pkcs11_enumerator
{
private:
    ///
    typedef std::vector<
        std::string
    > module_path_list;

     ///
    typedef std::function<
        void(const scard_x509_identity_list &)
    > handler;

public:
    ///
    scard_pkcs11_enumerator(module_path_list module_paths,
        handler handler);

    ///
    scard_x509_identity_list enumerate() const;

private:
    ///
    const module_path_list _module_paths;

    ///
    handler _handler;
};