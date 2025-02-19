// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN
// 

#pragma once

#include <cstdint>


template<class E> struct is_valid_enum_value {};

enum class ModuleName : uint8_t
{
    UNKNOWN = 0,
    login = 1,
    selector = 2,
    confirm = 3,
    challenge = 4,
    valid = 5,
    transitory = 6,
    close = 7,
    close_back = 8,
    interactive_target = 9,
    RDP = 10,
    VNC = 11,
    INTERNAL = 12,
    waitinfo = 13,
    bouncer2 = 14,
    autotest = 15,
    widgettest = 16,
    card = 17,
};

template<> struct is_valid_enum_value<ModuleName>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 17; }
};

// Specifies the type of data to be captured:
enum class CaptureFlags : uint8_t
{
    none = 0,
    png = 1,
    wrm = 2,
    video = 4,
    ocr = 8,
};

template<> struct is_valid_enum_value<CaptureFlags>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 15; }
};

inline CaptureFlags operator | (CaptureFlags x, CaptureFlags y)
{ return CaptureFlags(uint8_t(x) | uint8_t(y)); }
inline CaptureFlags operator & (CaptureFlags x, CaptureFlags y)
{ return CaptureFlags(uint8_t(x) & uint8_t(y)); }
inline CaptureFlags operator ~ (CaptureFlags x)
{ return CaptureFlags(~uint8_t(x) & uint8_t(15)); }
// inline CaptureFlags operator + (CaptureFlags x, CaptureFlags y) { return x | y; }
// inline CaptureFlags operator - (CaptureFlags x, CaptureFlags y) { return x & ~y; }
inline CaptureFlags & operator |= (CaptureFlags & x, CaptureFlags y) { return x = x | y; }
inline CaptureFlags & operator &= (CaptureFlags & x, CaptureFlags y) { return x = x & y; }
// inline CaptureFlags & operator += (CaptureFlags & x, CaptureFlags y) { return x = x + y; }
// inline CaptureFlags & operator -= (CaptureFlags & x, CaptureFlags y) { return x = x - y; }

enum class Level : uint8_t
{
    low = 0,
    medium = 1,
    high = 2,
};

template<> struct is_valid_enum_value<Level>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

enum class Language : uint8_t
{
    en = 0,
    fr = 1,
};

template<> struct is_valid_enum_value<Language>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 1; }
};

enum class ClipboardEncodingType : uint8_t
{
    utf8 = 0,
    latin1 = 1,
};

template<> struct is_valid_enum_value<ClipboardEncodingType>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 1; }
};

enum class KeyboardLogFlagsCP : uint8_t
{
    none = 0,
    // keyboard log in syslog
    syslog = 1,
    // keyboard log in recorded sessions
    wrm = 2,
};

template<> struct is_valid_enum_value<KeyboardLogFlagsCP>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 3; }
};

inline KeyboardLogFlagsCP operator | (KeyboardLogFlagsCP x, KeyboardLogFlagsCP y)
{ return KeyboardLogFlagsCP(uint8_t(x) | uint8_t(y)); }
inline KeyboardLogFlagsCP operator & (KeyboardLogFlagsCP x, KeyboardLogFlagsCP y)
{ return KeyboardLogFlagsCP(uint8_t(x) & uint8_t(y)); }
inline KeyboardLogFlagsCP operator ~ (KeyboardLogFlagsCP x)
{ return KeyboardLogFlagsCP(~uint8_t(x) & uint8_t(3)); }
// inline KeyboardLogFlagsCP operator + (KeyboardLogFlagsCP x, KeyboardLogFlagsCP y) { return x | y; }
// inline KeyboardLogFlagsCP operator - (KeyboardLogFlagsCP x, KeyboardLogFlagsCP y) { return x & ~y; }
inline KeyboardLogFlagsCP & operator |= (KeyboardLogFlagsCP & x, KeyboardLogFlagsCP y) { return x = x | y; }
inline KeyboardLogFlagsCP & operator &= (KeyboardLogFlagsCP & x, KeyboardLogFlagsCP y) { return x = x & y; }
// inline KeyboardLogFlagsCP & operator += (KeyboardLogFlagsCP & x, KeyboardLogFlagsCP y) { return x = x + y; }
// inline KeyboardLogFlagsCP & operator -= (KeyboardLogFlagsCP & x, KeyboardLogFlagsCP y) { return x = x - y; }

enum class KeyboardLogFlags : uint8_t
{
    none = 0,
    // keyboard log in syslog
    syslog = 1,
    // keyboard log in recorded sessions
    wrm = 2,
    // keyboard log in recorded meta
    meta = 4,
};

template<> struct is_valid_enum_value<KeyboardLogFlags>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 7; }
};

inline KeyboardLogFlags operator | (KeyboardLogFlags x, KeyboardLogFlags y)
{ return KeyboardLogFlags(uint8_t(x) | uint8_t(y)); }
inline KeyboardLogFlags operator & (KeyboardLogFlags x, KeyboardLogFlags y)
{ return KeyboardLogFlags(uint8_t(x) & uint8_t(y)); }
inline KeyboardLogFlags operator ~ (KeyboardLogFlags x)
{ return KeyboardLogFlags(~uint8_t(x) & uint8_t(7)); }
// inline KeyboardLogFlags operator + (KeyboardLogFlags x, KeyboardLogFlags y) { return x | y; }
// inline KeyboardLogFlags operator - (KeyboardLogFlags x, KeyboardLogFlags y) { return x & ~y; }
inline KeyboardLogFlags & operator |= (KeyboardLogFlags & x, KeyboardLogFlags y) { return x = x | y; }
inline KeyboardLogFlags & operator &= (KeyboardLogFlags & x, KeyboardLogFlags y) { return x = x & y; }
// inline KeyboardLogFlags & operator += (KeyboardLogFlags & x, KeyboardLogFlags y) { return x = x + y; }
// inline KeyboardLogFlags & operator -= (KeyboardLogFlags & x, KeyboardLogFlags y) { return x = x - y; }

enum class ClipboardLogFlags : uint8_t
{
    none = 0,
    // clipboard log in syslog
    syslog = 1,
    // clipboard log in recorded sessions
    wrm = 2,
    // clipboard log in recorded meta
    meta = 4,
};

template<> struct is_valid_enum_value<ClipboardLogFlags>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 7; }
};

inline ClipboardLogFlags operator | (ClipboardLogFlags x, ClipboardLogFlags y)
{ return ClipboardLogFlags(uint8_t(x) | uint8_t(y)); }
inline ClipboardLogFlags operator & (ClipboardLogFlags x, ClipboardLogFlags y)
{ return ClipboardLogFlags(uint8_t(x) & uint8_t(y)); }
inline ClipboardLogFlags operator ~ (ClipboardLogFlags x)
{ return ClipboardLogFlags(~uint8_t(x) & uint8_t(7)); }
// inline ClipboardLogFlags operator + (ClipboardLogFlags x, ClipboardLogFlags y) { return x | y; }
// inline ClipboardLogFlags operator - (ClipboardLogFlags x, ClipboardLogFlags y) { return x & ~y; }
inline ClipboardLogFlags & operator |= (ClipboardLogFlags & x, ClipboardLogFlags y) { return x = x | y; }
inline ClipboardLogFlags & operator &= (ClipboardLogFlags & x, ClipboardLogFlags y) { return x = x & y; }
// inline ClipboardLogFlags & operator += (ClipboardLogFlags & x, ClipboardLogFlags y) { return x = x + y; }
// inline ClipboardLogFlags & operator -= (ClipboardLogFlags & x, ClipboardLogFlags y) { return x = x - y; }

enum class FileSystemLogFlags : uint8_t
{
    none = 0,
    // (redirected) file system log in syslog
    syslog = 1,
    // (redirected) file system log in recorded sessions
    wrm = 2,
    // (redirected) file system log in recorded meta
    meta = 4,
};

template<> struct is_valid_enum_value<FileSystemLogFlags>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 7; }
};

inline FileSystemLogFlags operator | (FileSystemLogFlags x, FileSystemLogFlags y)
{ return FileSystemLogFlags(uint8_t(x) | uint8_t(y)); }
inline FileSystemLogFlags operator & (FileSystemLogFlags x, FileSystemLogFlags y)
{ return FileSystemLogFlags(uint8_t(x) & uint8_t(y)); }
inline FileSystemLogFlags operator ~ (FileSystemLogFlags x)
{ return FileSystemLogFlags(~uint8_t(x) & uint8_t(7)); }
// inline FileSystemLogFlags operator + (FileSystemLogFlags x, FileSystemLogFlags y) { return x | y; }
// inline FileSystemLogFlags operator - (FileSystemLogFlags x, FileSystemLogFlags y) { return x & ~y; }
inline FileSystemLogFlags & operator |= (FileSystemLogFlags & x, FileSystemLogFlags y) { return x = x | y; }
inline FileSystemLogFlags & operator &= (FileSystemLogFlags & x, FileSystemLogFlags y) { return x = x & y; }
// inline FileSystemLogFlags & operator += (FileSystemLogFlags & x, FileSystemLogFlags y) { return x = x + y; }
// inline FileSystemLogFlags & operator -= (FileSystemLogFlags & x, FileSystemLogFlags y) { return x = x - y; }

// Specifies the maximum color resolution (color depth) for client session:
enum class ColorDepth : uint8_t
{
    // 8-bit
    depth8 = 8,
    // 15-bit 555 RGB mask (5 bits for red, 5 bits for green, and 5 bits for blue)
    depth15 = 15,
    // 16-bit 565 RGB mask (5 bits for red, 6 bits for green, and 5 bits for blue)
    depth16 = 16,
    // 24-bit RGB mask (8 bits for red, 8 bits for green, and 8 bits for blue)
    depth24 = 24,
    // 32-bit RGB mask (8 bits for alpha, 8 bits for red, 8 bits for green, and 8 bits for blue)
    depth32 = 32,
};

template<> struct is_valid_enum_value<ColorDepth>
{
    constexpr static bool is_valid(uint64_t n)
    {
        switch (n) {
        case uint64_t(8):
        case uint64_t(15):
        case uint64_t(16):
        case uint64_t(24):
        case uint64_t(32):
            return true;
        }
        return false;
    }

};

enum class ServerNotification : uint8_t
{
    nobody = 0,
    // message sent to syslog
    syslog = 1,
    // User notified (through proxy interface)
    user = 2,
    // admin notified (wab notification)
    admin = 4,
};

template<> struct is_valid_enum_value<ServerNotification>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 7; }
};

inline ServerNotification operator | (ServerNotification x, ServerNotification y)
{ return ServerNotification(uint8_t(x) | uint8_t(y)); }
inline ServerNotification operator & (ServerNotification x, ServerNotification y)
{ return ServerNotification(uint8_t(x) & uint8_t(y)); }
inline ServerNotification operator ~ (ServerNotification x)
{ return ServerNotification(~uint8_t(x) & uint8_t(7)); }
// inline ServerNotification operator + (ServerNotification x, ServerNotification y) { return x | y; }
// inline ServerNotification operator - (ServerNotification x, ServerNotification y) { return x & ~y; }
inline ServerNotification & operator |= (ServerNotification & x, ServerNotification y) { return x = x | y; }
inline ServerNotification & operator &= (ServerNotification & x, ServerNotification y) { return x = x & y; }
// inline ServerNotification & operator += (ServerNotification & x, ServerNotification y) { return x = x + y; }
// inline ServerNotification & operator -= (ServerNotification & x, ServerNotification y) { return x = x - y; }

// Behavior of certificates check.
// System errors like FS access rights issues or certificate decode are always check errors leading to connection rejection.
enum class ServerCertCheck : uint8_t
{
    // fails if certificates doesn't match or miss.
    fails_if_no_match_or_missing = 0,
    // fails if certificate doesn't match, succeed if no known certificate.
    fails_if_no_match_and_succeed_if_no_know = 1,
    // succeed if certificates exists (not checked), fails if missing.
    succeed_if_exists_and_fails_if_missing = 2,
    // always succeed.
    always_succeed = 3,
};

template<> struct is_valid_enum_value<ServerCertCheck>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 3; }
};

// Session record options.
// When session records are encrypted, they can be read only by the WAB where they have been generated.
enum class TraceType : uint8_t
{
    // No encryption (faster).
    localfile = 0,
    // No encryption, with checksum.
    localfile_hashed = 1,
    // Encryption enabled.
    cryptofile = 2,
};

template<> struct is_valid_enum_value<TraceType>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

enum class KeyboardInputMaskingLevel : uint8_t
{
    // keyboard input are not masked
    unmasked = 0,
    // only passwords are masked
    password_only = 1,
    // passwords and unidentified texts are masked
    password_and_unidentified = 2,
    // keyboard inputs are not logged
    fully_masked = 3,
};

template<> struct is_valid_enum_value<KeyboardInputMaskingLevel>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 3; }
};

// Behavior on failure to launch Session Probe.
enum class SessionProbeOnLaunchFailure : uint8_t
{
    // ignore failure and continue.
    ignore_and_continue = 0,
    // disconnect user.
    disconnect_user = 1,
    // reconnect without Session Probe.
    retry_without_session_probe = 2,
};

template<> struct is_valid_enum_value<SessionProbeOnLaunchFailure>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

enum class VncBogusClipboardInfiniteLoop : uint8_t
{
    delayed = 0,
    duplicated = 1,
    continued = 2,
};

template<> struct is_valid_enum_value<VncBogusClipboardInfiniteLoop>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

// The method by which the proxy RDP establishes criteria on which to chosse a color depth for native video capture:
enum class ColorDepthSelectionStrategy : uint8_t
{
    // 24-bit
    depth24 = 0,
    // 16-bit
    depth16 = 1,
};

template<> struct is_valid_enum_value<ColorDepthSelectionStrategy>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 1; }
};

// The compression method of native video capture:
enum class WrmCompressionAlgorithm : uint8_t
{
    no_compression = 0,
    gzip = 1,
    snappy = 2,
};

template<> struct is_valid_enum_value<WrmCompressionAlgorithm>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

// Specifies the highest compression package support available on the front side
enum class RdpCompression : uint8_t
{
    // The RDP bulk compression is disabled
    none = 0,
    // RDP 4.0 bulk compression
    rdp4 = 1,
    // RDP 5.0 bulk compression
    rdp5 = 2,
    // RDP 6.0 bulk compression
    rdp6 = 3,
    // RDP 6.1 bulk compression
    rdp6_1 = 4,
};

template<> struct is_valid_enum_value<RdpCompression>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 4; }
};

enum class OcrVersion : uint8_t
{
    v1 = 1,
    v2 = 2,
};

template<> struct is_valid_enum_value<OcrVersion>
{
    constexpr static bool is_valid(uint64_t n)
    {
        switch (n) {
        case uint64_t(1):
        case uint64_t(2):
            return true;
        }
        return false;
    }

};

enum class OcrLocale : uint8_t
{
    latin = 0,
    cyrillic = 1,
};

template<> struct is_valid_enum_value<OcrLocale>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 1; }
};

enum class BogusNumberOfFastpathInputEvent : uint8_t
{
    disabled = 0,
    pause_key_only = 1,
    all_input_events = 2,
};

template<> struct is_valid_enum_value<BogusNumberOfFastpathInputEvent>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

enum class SessionProbeOnKeepaliveTimeout : uint8_t
{
    ignore_and_continue = 0,
    disconnect_user = 1,
    freeze_connection_and_wait = 2,
};

template<> struct is_valid_enum_value<SessionProbeOnKeepaliveTimeout>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

enum class SmartVideoCropping : uint8_t
{
    // Disabled. When replaying the session video, the content of the RDP viewer matches the size of the client's desktop
    disable = 0,
    // When replaying the session video, the content of the RDP viewer is restricted to the greatest area covered by the application during session
    v1 = 1,
    // When replaying the session video, the content of the RDP viewer is fully covered by the size of the greatest application window during session
    v2 = 2,
};

template<> struct is_valid_enum_value<SmartVideoCropping>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

enum class RdpModeConsole : uint8_t
{
    // Forward Console mode request from client to the target.
    allow = 0,
    // Force Console mode on target regardless of client request.
    force = 1,
    // Block Console mode request from client.
    forbid = 2,
};

template<> struct is_valid_enum_value<RdpModeConsole>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

enum class SessionProbeDisabledFeature : uint16_t
{
    none = 0,
    // Java Access Bridge
    jab = 1,
    // MS Active Accessbility
    msaa = 2,
    // MS UI Automation
    msuia = 4,
    // Reserved (do not use)
    r1 = 8,
    // Inspect Edge location URL
    edge_inspection = 16,
    // Inspect Chrome Address/Search bar
    chrome_inspection = 32,
    // Inspect Firefox Address/Search bar
    firefox_inspection = 64,
    // Monitor Internet Explorer event
    ie_monitoring = 128,
    // Inspect group membership of user
    group_membership = 256,
};

template<> struct is_valid_enum_value<SessionProbeDisabledFeature>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 511; }
};

inline SessionProbeDisabledFeature operator | (SessionProbeDisabledFeature x, SessionProbeDisabledFeature y)
{ return SessionProbeDisabledFeature(uint16_t(x) | uint16_t(y)); }
inline SessionProbeDisabledFeature operator & (SessionProbeDisabledFeature x, SessionProbeDisabledFeature y)
{ return SessionProbeDisabledFeature(uint16_t(x) & uint16_t(y)); }
inline SessionProbeDisabledFeature operator ~ (SessionProbeDisabledFeature x)
{ return SessionProbeDisabledFeature(~uint16_t(x) & uint16_t(511)); }
// inline SessionProbeDisabledFeature operator + (SessionProbeDisabledFeature x, SessionProbeDisabledFeature y) { return x | y; }
// inline SessionProbeDisabledFeature operator - (SessionProbeDisabledFeature x, SessionProbeDisabledFeature y) { return x & ~y; }
inline SessionProbeDisabledFeature & operator |= (SessionProbeDisabledFeature & x, SessionProbeDisabledFeature y) { return x = x | y; }
inline SessionProbeDisabledFeature & operator &= (SessionProbeDisabledFeature & x, SessionProbeDisabledFeature y) { return x = x & y; }
// inline SessionProbeDisabledFeature & operator += (SessionProbeDisabledFeature & x, SessionProbeDisabledFeature y) { return x = x + y; }
// inline SessionProbeDisabledFeature & operator -= (SessionProbeDisabledFeature & x, SessionProbeDisabledFeature y) { return x = x - y; }

enum class RdpStoreFile : uint8_t
{
    // Never store transferred files.
    never = 0,
    // Always store transferred files.
    always = 1,
    // Transferred files are stored only if file verification is invalid. File verification by ICAP service must be enabled (in section file_verification).
    on_invalid_verification = 2,
};

template<> struct is_valid_enum_value<RdpStoreFile>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

// For targets running WALLIX BestSafe only.
enum class SessionProbeOnAccountManipulation : uint8_t
{
    // User action will be accepted
    allow = 0,
    // (Same thing as 'allow') 
    notify = 1,
    // User action will be rejected
    deny = 2,
};

template<> struct is_valid_enum_value<SessionProbeOnAccountManipulation>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

// Client Address to send to target(in InfoPacket)
enum class ClientAddressSent : uint8_t
{
    // Send 0.0.0.0
    no_address = 0,
    // Send proxy client address or target connexion
    proxy = 1,
    // Send user client address of front connexion
    front = 2,
};

template<> struct is_valid_enum_value<ClientAddressSent>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

enum class SessionProbeLogLevel : uint8_t
{
    Off = 0,
    Fatal = 1,
    Error = 2,
    Info = 3,
    Warning = 4,
    Debug = 5,
    Detail = 6,
};

template<> struct is_valid_enum_value<SessionProbeLogLevel>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 6; }
};

enum class ModRdpUseFailureSimulationSocketTransport : uint8_t
{
    Off = 0,
    SimulateErrorRead = 1,
    SimulateErrorWrite = 2,
};

template<> struct is_valid_enum_value<ModRdpUseFailureSimulationSocketTransport>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

enum class LoginLanguage : uint8_t
{
    Auto = 0,
    EN = 1,
    FR = 2,
};

template<> struct is_valid_enum_value<LoginLanguage>
{
    constexpr static bool is_valid(uint64_t n) { return n <= 2; }
};

