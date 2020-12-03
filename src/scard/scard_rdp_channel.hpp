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
#include <memory>
#include <unordered_map>

#include "mod/rdp/channels/base_channel.hpp"
#include "scard/scard_pcsc_common.hpp"
#include "scard/scard_utils.hpp"
#include "utils/sugar/bytes_view.hpp"

class EventContainer;
class FileSystemDriveManager;
class FileSystemVirtualChannel;
class FileSystemVirtualChannelParams;
class TimeBase;
class VirtualChannelDataSender;


///////////////////////////////////////////////////////////////////////////////


class SCardChannel final : public BaseVirtualChannel
{
public:
    typedef std::function<void(const scard_wrapped_return&)> IncomingDataHandler;

public:
    ///
    SCardChannel(
        const BaseVirtualChannel::Params &base_params,
        const FileSystemVirtualChannelParams &fs_params,
        VirtualChannelDataSender *to_client_sender,
        TimeBase &time_base,
        EventContainer &events,
        IncomingDataHandler handler);

    ///
    virtual ~SCardChannel();

    ///
    virtual void process_client_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override;

    ///
    virtual void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data,
        std::unique_ptr<AsynchronousTask> &out_asynchronous_task) override;

    ///
    void send_call(const scard_wrapped_call &wrapped_call);

private:
    ///
    void announce();

    ///
    void process_client_announce_reply(InStream &chunk, uint32_t flags);

    ///
    void process_client_name_request(InStream &chunk, uint32_t flags);

    ///
    void process_client_core_capability_response(InStream &chunk, uint32_t flags);

    ///
    void process_client_device_list_announce_request(InStream &chunk, uint32_t flags);

    ///
    void send_server_core_capability_request();

    ///
    void send_server_client_id_confirm();

    ///
    void send_device_io_request(uint32_t io_control_code, bytes_view data);

    ///
    void process_device_control_response(InStream &chunk, uint32_t flags);

    ///
    std::unique_ptr<FileSystemVirtualChannel> _transport_channel;

    ///
    std::unique_ptr<FileSystemDriveManager> _drive_manager;

    /// Packet header.
    rdpdr::SharedHeader _packet_header;

    /// Client IDentifier sent by the client in the Client Announce Reply message.
    // FIXME Should be uint32_t according to MS specifications
    uint16_t _client_id;

    /// Handler for data coming from RDP client.
    IncomingDataHandler _handler;

    /// Handle table used for managing completion IDs.
    static_handle_table<uint32_t, 255, 0> _handles;

    /// IO requests.
    std::unordered_map<
        uint32_t, uint32_t
    > _requests;
};