#include <functional>
#include <memory>
#include <stdexcept>

#include "acl/auth_api.hpp"
#include "core/channel_list.hpp"
#include "core/file_system_virtual_channel_params.hpp"
#include "core/front_api.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "scard/scard_manager.hpp"
#include "scard/scard_pcsc_common.hpp"
#include "scard/scard_pkcs11_enumerator.hpp"
#include "scard/scard_rdp_channel.hpp"
#include "utils/log.hpp"


///////////////////////////////////////////////////////////////////////////////


scard_manager::scard_manager(
    AuthApi& authenticator,
    FrontAPI& front,
    TimeBase& time_base,
    EventContainer& events)
{
    // create front sender
    {
        const CHANNELS::ChannelDefArray& channel_list = front.get_channel_list();
        const CHANNELS::ChannelDef* channel = channel_list.get_by_name(
            CHANNELS::channel_names::rdpdr);

        if (!channel)
        {
            throw std::runtime_error(
                "Could not find scard transport channel among the available front channels.");
        }

        _front_sender = std::make_unique<_FrontSender>(front, *channel, true);
    }

    // create channel
    {
        BaseVirtualChannel::Params base_params(authenticator,
            RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump);
        FileSystemVirtualChannelParams fs_params;
        fs_params.smart_card_authorized = true;

        _channel = std::make_unique<SCardChannel>(base_params, fs_params,
            _front_sender.get(), time_base, events,
            std::bind(&scard_manager::handle_scard_return, this, std::placeholders::_1)
        );
    }
    
    // create server
    {
        _server = std::make_unique<scard_pcsc_server>("/tmp/rdpcsc.socket",
            std::bind(&scard_manager::handle_scard_call, this, std::placeholders::_1)
        );
    }
    
    // create PKCS#11 enumerator
    {
        _enumerator = std::make_unique<scard_pkcs11_enumerator>(
            std::vector<std::string>({
                //"/usr/lib/pkcs11/libeToken.so",
                "/usr/local/lib/opensc-pkcs11.so"
            }),
            std::bind(&scard_manager::handle_discovered_identities, this, std::placeholders::_1)
        );
    }

    // spawn server and enumerator threads
    _server_thread = std::thread(
        std::bind(&scard_pcsc_server::serve, _server.get())
    );
    _enumerator_thread = std::thread(
        std::bind(&scard_pkcs11_enumerator::enumerate, _enumerator.get())
    );
}

scard_manager::~scard_manager()
{
    if (_server)
    {
        _server->stop();
    }
    if (_enumerator)
    {
        // TODO
    }

    _server_thread.join();
    _enumerator_thread.join();
}

void scard_manager::process_incoming_message(uint32_t total_length,
    uint32_t flags, bytes_view chunk_data)
{
    _channel->process_client_message(total_length, flags, chunk_data);
}

void scard_manager::handle_scard_call(const scard_wrapped_call &wrapped_call)
{
    _channel->send_call(wrapped_call);
}

void scard_manager::handle_scard_return(const scard_wrapped_return &wrapped_return)
{
    _server->post_return(wrapped_return);
}

void scard_manager::handle_discovered_identities(
    const scard_x509_identity_list& identities)
{
    LOG(LOG_DEBUG, "scard_manager: "
        "Discovered %" PRIu64 " identities.",
        identities.size());
}
