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

#include <memory>
#include <thread>

#include "acl/auth_api.hpp"
#include "core/channel_list.hpp"
#include "core/front_api.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "scard/scard_pcsc_common.hpp"
#include "scard/scard_pkcs11_enumerator.hpp"
#include "scard/scard_pcsc_server.hpp"
#include "scard/scard_rdp_channel.hpp"
#include "scard/scard_x509.hpp"
#include "utils/sugar/bytes_view.hpp"


///////////////////////////////////////////////////////////////////////////////


class scard_manager
{
public:
    ///
    scard_manager(
        AuthApi& authenticator,
        FrontAPI& front,
        TimeBase& time_base,
        EventContainer& events
    );

    ///
    ~scard_manager();
    
    ///
    void process_incoming_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data);

private:
    ///
    class _FrontSender : public VirtualChannelDataSender
    {
    public:
        explicit _FrontSender(FrontAPI& front, const CHANNELS::ChannelDef& channel,
            bool verbose) noexcept
        : _front(front)
        , _channel(channel)
        , _verbose(verbose)
        {}

        void operator()(uint32_t total_length, uint32_t flags,
            bytes_view chunk_data) override
        {
            if (_verbose) {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, total_length, flags, chunk_data);
            }

            _front.send_to_channel(_channel, chunk_data, total_length, flags);
        }

    private:
        ///
        FrontAPI& _front;

        ///
        const CHANNELS::ChannelDef& _channel;

        ///
        const bool _verbose;
    };

    ///
    std::unique_ptr<_FrontSender> _front_sender;

    ///
    std::unique_ptr<SCardChannel> _channel;

    ///
    std::unique_ptr<scard_pcsc_server> _server;

    ///
    std::unique_ptr<scard_pkcs11_enumerator> _enumerator;

    ///
    std::thread _server_thread;

    ///
    std::thread _enumerator_thread;

    ///
    void handle_scard_call(const scard_wrapped_call &wrapped_call);

    ///
    void handle_scard_return(const scard_wrapped_return &wrapped_return);

    ///
    void handle_discovered_identities(
        const scard_x509_identity_list& identities);
};