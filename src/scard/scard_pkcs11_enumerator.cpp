#include <cinttypes>
#include <string>
#include <vector>

#include "scard/scard_pkcs11_enumerator.hpp"
#include "scard/scard_pkcs11_module.hpp"
#include "scard/scard_utils.hpp"
#include "scard/scard_x509.hpp"
#include "utils/log.hpp"


///////////////////////////////////////////////////////////////////////////////


scard_pkcs11_enumerator::scard_pkcs11_enumerator(module_path_list module_paths,
    handler handler)
    :
    _module_paths(std::move(module_paths)),
    _handler(std::move(handler))
{
}

scard_x509_identity_list scard_pkcs11_enumerator::enumerate() const
{
    scard_x509_identity_list identities;

    for (const auto &module_path : _module_paths)
    {
        LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
            "loading PKCS#11 module '%s'", module_path.c_str());

        // load module
        auto module = scard_pkcs11_module::load(module_path);

        LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
            "PKCS#11 module loaded.");

        LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
            "listing slots with token...");

        // list slots
        auto slots = module.list_slots(true);

        LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
            "found %" PRIu64 " slot(s) with token.",
            slots.size());

        // skip this module if no suitable slot was found
        if (slots.empty())
        {
            continue;
        }

        for (auto &slot : slots)
        {
            LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                "selected slot #%" PRIu64 ".",
                slot.get_id());
            
            LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                "retrieving slot/token information...");

            // describe slot
            module.describe_slot(slot);

            // describe token
            module.describe_token(slot);

            LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                "Slot [ID=%" PRIu64 " description='%s']",
                slot.get_id(), slot.get_description());
        }

        for (auto &slot : slots)
        {
            LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                "opening session against slot #%" PRIu64 "...",
                slot.get_id());

            // open session against the current slot
            const auto &session = module.open_session(slot);

            LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                "session opened.");

            std::vector<uint8_t> pin = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36 };

            LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                "logging in as user with PIN...");

            // login as user with PIN
            const auto &authenticated = module.login(session,
                writable_bytes_view(pin.data(), pin.size()));
            if (!authenticated)
            {
                LOG(LOG_ERR, "scard_pkcs11_enumerator::enumerate: "
                    "login failed.");
                
                continue;
            }

            LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                "logged in.");

            LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                "searching for X.509 signing certificates on-card...");

            // find certificates
            const auto &certificates = module.find_signing_certificates(session);

            LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                "found %" PRIu64 " suitable certificate(s).",
                certificates.size());

            for (const auto &certificate : certificates)
            {
                LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                    "Certificate [ID='%s' label='%s' type='%s' category='%s'].",
                    certificate.get_id_as_string(":"), certificate.get_label(),
                    certificate.get_type_as_string(), certificate.get_category_as_string());
                
                LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                    "searching for X.509 names in certificate...");
                
                // extract names from the X.509 subjectAltName certificate extension
                const auto names = extract_subject_alt_names(certificate.get_data());

                LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                    "found %" PRIu64 " X.509 name(s) in certificate.",
                    names.size());

                for (const auto &name : names)
                {
                    LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
                        "X.509 name [type='%s' value='%s'].",
                        name_type_to_string(name.type), name.value);

                    // save identity
                    if (name.type == scard_x509_name_type::ms_upn)
                    {
                        identities.push_back(scard_x509_identity(
                            module_path, std::to_string(slot.get_id()), slot.get_token_label(),
                            certificate.get_id_as_string()
                        ));
                    }
                }
            }
        }       
    }

    LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
            "found %" PRIu64 " X.509 identities on-card.",
            identities.size());

    for (const auto &identity : identities)
    {
        LOG(LOG_DEBUG, "scard_pkcs11_enumerator::enumerate: "
            "X.509 identity [value='%s'].",
            identity.string());
    }

    // handle enumerated identities
    if (_handler)
    {
        _handler(identities);
    }

    return identities;
}