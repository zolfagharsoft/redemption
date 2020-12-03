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

#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "utils/sugar/unique_fd.hpp"
#include "scard/scard_pcsc_common.hpp"


///////////////////////////////////////////////////////////////////////////////


class scard_pcsc_server
{
public:
    ///
    typedef std::function<void(const scard_wrapped_call&)> handler_type;

    ///
    explicit scard_pcsc_server(const std::string &path,
        handler_type handler);

    ///
    ~scard_pcsc_server();

    ///
    void serve();

    ///
    void stop();

    ///
    void post_return(scard_wrapped_return wrapped_return);

private:
    ///
    scard_wrapped_call receive(int fd, uint8_t *buffer, std::size_t buffer_size);

    ///
    void send(int fd, scard_wrapped_return &&wrapped_return);

    ///
    bool _state;

    ///
    unique_fd _server_socket;

    ///
    handler_type _handler;

    ///
    std::deque<scard_wrapped_return> _returns;

    ///
    std::mutex _state_mutex;

    ///
    std::mutex _returns_mutex;
};