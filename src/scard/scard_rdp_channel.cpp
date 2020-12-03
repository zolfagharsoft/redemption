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

#include <cassert>
#include <cinttypes>
#include <memory>
#include <string>

#include "core/channel_list.hpp"
#include "core/events.hpp"
#include "core/file_system_virtual_channel_params.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/rdpdr_channel.hpp"
#include "mod/rdp/channels/rdpdr_file_system_drive_manager.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "utils/stream.hpp"

#include "scard/scard_rdp_channel.hpp"


///////////////////////////////////////////////////////////////////////////////


enum
{
    common_type_header_version      = 0x01,
    common_type_header_endianness   = 0x10,
    common_type_header_length       = 8,
    common_type_header_filler       = 0xCCCCCCCC,
    private_type_header_length      = 8,
    private_type_header_filler      = 0x00000000
};

SCardChannel::SCardChannel(
    const BaseVirtualChannel::Params &base_params,
    const FileSystemVirtualChannelParams &fs_params,
    VirtualChannelDataSender *to_client_sender,
    TimeBase &time_base, EventContainer &events,
    IncomingDataHandler handler)
    :
    BaseVirtualChannel(to_client_sender, nullptr, base_params),
    _client_id(0),
    _handler(handler)
{

    _drive_manager = std::make_unique<FileSystemDriveManager>();
    _transport_channel = std::make_unique<FileSystemVirtualChannel>(
        time_base,
        events,
        to_client_sender,
        nullptr,
        *_drive_manager,
        false,
        std::string(""),
        "",
        16,
        "",
        base_params,
        fs_params
    );

    // send Server Announce Request
    announce();
}

SCardChannel::~SCardChannel()
{
}

void SCardChannel::send_call(const scard_wrapped_call& wrapped_call)
{
    send_device_io_request(wrapped_call.io_control_code,
        wrapped_call.data);
}

void SCardChannel::process_client_message(uint32_t total_length,
    uint32_t flags, bytes_view chunk_data)
{
    LOG_IF(bool(verbose & RDPVerbose::rdpdr), LOG_INFO,
        "SCardChannel::process_client_message:"
        " total_length=%" PRIu32 " flags=0x%08X chunk_data_length=%zu",
        total_length, flags, chunk_data.size());

    if (bool(verbose & RDPVerbose::rdpdr_dump))
    {
        ::msgdump_c(false, true, total_length, flags, chunk_data);
    }

    InStream chunk(chunk_data);

    // receive packet header
    if (flags & CHANNELS::CHANNEL_FLAG_FIRST)
    {
        _packet_header.receive(chunk);
    }

    // receive packet body
    switch (_packet_header.packet_id)
    {
        case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
        {
            process_client_announce_reply(chunk, flags);
        } break;

        case rdpdr::PacketId::PAKID_CORE_CLIENT_NAME:
        {
            process_client_name_request(chunk, flags);
        } break;

        case rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY:
        {
            process_client_core_capability_response(chunk, flags);
        } break;

        case rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE:
        {
            process_client_device_list_announce_request(chunk, flags);
        } break;

        case rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION:
        {
            process_device_control_response(chunk, flags);
        } break;

        default:
            break;
    }
}

void SCardChannel::process_server_message(uint32_t /*total_length*/,
    uint32_t /*flags*/, bytes_view /*chunk_data*/,
    std::unique_ptr<AsynchronousTask>& /*out_asynchronous_task*/)
{
}

void SCardChannel::announce()
{
    StaticOutStream<64> stream;

    rdpdr::SharedHeader header(
        rdpdr::Component::RDPDR_CTYP_CORE,
        rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE);
    header.emit(stream);

    rdpdr::ServerAnnounceRequest announce_request(
        0x0001,     // VersionMajor
        0x000D,     // VersionMinor
        9           // ClientID
    );
    announce_request.emit(stream);

    // send message
    _transport_channel->send_message_to_client(
        stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_produced_bytes());
}

void SCardChannel::process_client_announce_reply(InStream& chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

    if(bool(verbose & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
            "SCardChannel::process_client_announce_reply: "
            "Client Announce Reply");
    }

    rdpdr::ClientAnnounceReply client_announce_reply;

    // receive packet body
    client_announce_reply.receive(chunk);

    if (bool(verbose & RDPVerbose::rdpdr_dump))
    {
        client_announce_reply.log(LOG_INFO);
    }

    // save ClientId for later use
    _client_id = client_announce_reply.ClientId;
}

void SCardChannel::process_client_name_request(InStream& chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

    if (bool(verbose & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
            "SCardChannel::process_client_name_request: "
                "Client Name Request");
    }

    rdpdr::ClientNameRequest client_name_request;

    // receive packet body
    client_name_request.receive(chunk);

    if (bool(verbose & RDPVerbose::rdpdr_dump))
    {
        client_name_request.log(LOG_INFO);
    }

    // send Server Core Capability Request message
    send_server_core_capability_request();

    // send Server Client ID Confirm message
    send_server_client_id_confirm();
}

void SCardChannel::process_client_core_capability_response(InStream& chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));
    
    // numCapabilities(2) + Padding(2)
    ::check_throw(chunk, 4,
        "SCardChannel::process_client_core_capability_response:DR_CORE_CAPABILITY_RSP (1)",
        ERR_RDP_DATA_TRUNCATED);

    const uint16_t num_capabilities = chunk.in_uint16_le();

    if (bool(verbose & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
            "SCardChannel::process_client_core_capability_response: "
            "numCapabilities=%u", num_capabilities);
    }
}

void SCardChannel::process_client_device_list_announce_request(
    InStream& chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

    // DeviceCount(4)
    ::check_throw(chunk, 4,
        "SCardChannel::process_client_device_list_announce_request",
        ERR_RDP_DATA_TRUNCATED);

    const uint32_t device_count = chunk.in_uint32_le();

    if (bool(verbose & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
            "SCardChannel::process_client_device_list_announce_request: "
                "DeviceCount=%" PRIu32, device_count);
    }

    // TODO Send Server Device Announce Response messages
}

void SCardChannel::send_server_core_capability_request()
{
    StaticOutStream<1024> stream;

    rdpdr::SharedHeader header(
        rdpdr::Component::RDPDR_CTYP_CORE,
        rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY);
    header.emit(stream);

    rdpdr::ServerCoreCapabilityRequest server_core_capability_request(1);
    server_core_capability_request.emit(stream);

    // FIXME How about handling this in the ServerCoreCapabilityRequest class?
    const uint32_t general_capability_version = rdpdr::GENERAL_CAPABILITY_VERSION_02;
    stream.out_uint16_le(rdpdr::CAP_GENERAL_TYPE);
    stream.out_uint16_le(
        rdpdr::GeneralCapabilitySet::size(general_capability_version) +
        8   // CapabilityType(2) + CapabilityLength(2) + Version(4)
    );
    stream.out_uint32_le(general_capability_version);

    rdpdr::GeneralCapabilitySet general_capability_set(
        0x2,        // osType
        // 0x50001,                               // osVersion
        // 0x1,                                   // protocolMajorVersion
        0xc,                                    //this->serverVersionMinor,         // protocolMinorVersion -
                    //     RDP Client 6.0 and 6.1
        0xFFFF,     // ioCode1
        // 0x0,                                   // ioCode2
        0x7,        // extendedPDU -
                    //     RDPDR_DEVICE_REMOVE_PDUS(1) |
                    //     RDPDR_CLIENT_DISPLAY_NAME_PDU(2) |
                    //     RDPDR_USER_LOGGEDON_PDU(4)
        0x0,        // extraFlags1
        // 0x0,                                   // extraFlags2
        0,                                      // SpecialTypeDeviceCap
        general_capability_version
    );
    if (bool(verbose & RDPVerbose::rdpdr)) {
        LOG(LOG_INFO,
            "SCardChannel::send_server_core_capability_request: "
        );
        general_capability_set.log(LOG_INFO);
    }
    general_capability_set.emit(stream);

    // Smart card capability set
    stream.out_uint16_le(rdpdr::CAP_SMARTCARD_TYPE);
    stream.out_uint16_le(
        8   // CapabilityType(2) + CapabilityLength(2) +
            //     Version(4)
    );

    // send message over RDPDR virtual channel
    _transport_channel->send_message_to_client(
        stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_produced_bytes());
}

void SCardChannel::send_server_client_id_confirm()
{
    StaticOutStream<1024> stream;

    rdpdr::SharedHeader header(
        rdpdr::Component::RDPDR_CTYP_CORE,
        rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
    header.emit(stream);
    
    stream.out_uint16_le(0x0001);   // VersionMajor (must be set to 0x0001)
    stream.out_uint16_le(0x000C);   // VersionMinor (must be set to either 0x0002, 0x0005, 0x000A, 0x000C, 0x000D)
    stream.out_uint32_le(_client_id);

    // send message over RDPDR virtual channel
    _transport_channel->send_message_to_client(
        stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_produced_bytes());
}

void SCardChannel::send_device_io_request(uint32_t io_control_code, bytes_view data)
{
    StaticOutStream<1024> stream;

    // encode header
    {
        rdpdr::SharedHeader header(
            rdpdr::Component::RDPDR_CTYP_CORE,
            rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
        
        // do encode
        header.emit(stream); // 4 bytes
    }

    // encode Device IO Request
    {
        const uint32_t device_id = 0x0001;
        const uint32_t file_id = 0;
        const uint32_t completion_id = _handles.allocate();
        const uint32_t major_function = rdpdr::IRP_MJ_DEVICE_CONTROL;
        const uint32_t minor_function = 0x00000000; // must be set to this fixed value

        rdpdr::DeviceIORequest device_io_request(
            device_id, file_id, completion_id,
            major_function, minor_function
        );

        // do encode
        device_io_request.emit(stream); // 20 bytes

        // store the request
        const auto result =_requests.emplace(completion_id, io_control_code);

        assert(result.second);
    }

    // OutputBufferLength (4 bytes)
    stream.out_uint32_le(0x00000100);

    const uint32_t input_buffer_length = (
        common_type_header_length +     // CommonTypeHeader
        private_type_header_length +    // PrivateTypeHeader
        data.size()                     // ObjectBuffer
    );

    // InputBufferLength (4 bytes)
    stream.out_uint32_le(input_buffer_length);

    // IoControlCode (4 bytes)
    stream.out_uint32_le(io_control_code);

    // Padding (20 bytes)
    stream.out_skip_bytes(20);

    /* InputBuffer */

    /* CommonTypeHeader (8 bytes) */

    // Version (1 byte)
    stream.out_uint8(common_type_header_version);

    // Endianness (1 byte)
    stream.out_uint8(common_type_header_endianness);

    // CommonHeaderLength (2 bytes)
    stream.out_uint16_le(common_type_header_length);

    // Filler (4 bytes)
    stream.out_uint32_le(common_type_header_filler);

    /* PrivateTypeHeader (8 bytes + padding size) */

    // ObjectBufferLength (4 bytes)
    stream.out_uint32_le(data.size());

    // Filler (4 bytes)
    stream.out_uint32_le(private_type_header_filler);

    // Padding
    /*
    {
        const auto data_size = stream.get_offset();

        // align on a 8-byte boundary
        auto padding_size = ((data_size % 8) ? (8 - (data_size % 8)) : 0);
        while (padding_size--)
        {
            stream.out_uint8(0x00);
        }
    }
    */

    /* Payload */
    stream.out_copy_bytes(data);

    // Padding
    /*
    {
        const auto data_size = stream.get_offset();

        // pad (alignment on an 8-byte boundary)
        auto padding_size = ((data_size % 8) ? (8 - (data_size % 8)) : 0);
        while (padding_size--)
        {
            stream.out_uint8(0x00);
        }
    }
    */

    // send message over RDPDR virtual channel
    _transport_channel->send_message_to_client(
        stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_produced_bytes());
}

void SCardChannel::process_device_control_response(InStream& chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

    if (bool(verbose & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
        "scard_channel::process_device_control_response: "
            "Device Control Response");
    }

    uint32_t io_control_code;

    // receive and check Device IO Response
    {
        rdpdr::DeviceIOResponse device_io_response;

        device_io_response.receive(chunk);

        // check device ID
        {
            const uint32_t device_id = device_io_response.DeviceId();

            // TODO
        }

        // check completion ID and get the corresponding IO control code
        {
            const uint32_t completion_id = device_io_response.CompletionId();

            // ensure a request associated to this completion ID exists
            auto it = _requests.find(completion_id);
            if (it == _requests.end())
            {
                return;
            }

            // save IO control code
            io_control_code = it->second;

            // remove request
            _requests.erase(it);
        }

        // check IO status
        {
            const auto io_status = device_io_response.IoStatus();

            // TODO
        }
    }

    // OutputBufferLength
    const auto output_buffer_length = chunk.in_uint32_le();

    ::check_throw(chunk, output_buffer_length,
        "scard_channel::process_device_control_response: ",
        ERR_RDP_DATA_TRUNCATED);
    
    /* CommonTypeHeader (8 bytes) */

    // Version (1 byte)
    const auto version = chunk.in_uint8();

    assert(version == common_type_header_version);

    // Endianness (1 byte)
    const auto endianness = chunk.in_uint8();

    assert(endianness == common_type_header_endianness);

    // CommonHeaderLength (2 bytes)
    const auto length = chunk.in_uint16_le();

    assert(length == common_type_header_length);

    // Filler (4 bytes)
    chunk.in_skip_bytes(4);

    /* PrivateTypeHeader (8 bytes) */

    // ObjectBufferLength (4 bytes)
    const auto object_buffer_length = chunk.in_uint32_le();

    // Filler (4 bytes)
    chunk.in_skip_bytes(4);

    // Padding
    /*
    {
        const auto data_size = chunk.get_offset();

        // unpad (alignment on an 8-byte boundary)
        const auto padding_size = ((data_size % 8) ? (8 - (data_size % 8)) : 0);
        chunk.in_skip_bytes(padding_size);
    }
    */

    // defer handling of data
    if (_handler)
    {
        _handler(scard_wrapped_return(
            io_control_code,
            bytes_view(chunk.get_current(), object_buffer_length)
        ));
    }
}