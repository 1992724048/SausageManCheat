//
//  httplib.h
//
//  Copyright (c) 2025 Yuji Hirose. All rights reserved.
//  MIT License
//

#ifndef CPPHTTPLIB_HTTPLIB_H
#define CPPHTTPLIB_HTTPLIB_H

#define CPPHTTPLIB_VERSION "0.19.0"

/*
 * Configuration
 */

#ifndef CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_KEEPALIVE_TIMEOUT_CHECK_INTERVAL_USECOND
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_CHECK_INTERVAL_USECOND 10000
#endif

#ifndef CPPHTTPLIB_KEEPALIVE_MAX_COUNT
#define CPPHTTPLIB_KEEPALIVE_MAX_COUNT 1024
#endif

#ifndef CPPHTTPLIB_CONNECTION_TIMEOUT_SECOND
#define CPPHTTPLIB_CONNECTION_TIMEOUT_SECOND 300
#endif

#ifndef CPPHTTPLIB_CONNECTION_TIMEOUT_USECOND
#define CPPHTTPLIB_CONNECTION_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND
#define CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_SERVER_READ_TIMEOUT_USECOND
#define CPPHTTPLIB_SERVER_READ_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_SERVER_WRITE_TIMEOUT_SECOND
#define CPPHTTPLIB_SERVER_WRITE_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_SERVER_WRITE_TIMEOUT_USECOND
#define CPPHTTPLIB_SERVER_WRITE_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_CLIENT_READ_TIMEOUT_SECOND
#define CPPHTTPLIB_CLIENT_READ_TIMEOUT_SECOND 300
#endif

#ifndef CPPHTTPLIB_CLIENT_READ_TIMEOUT_USECOND
#define CPPHTTPLIB_CLIENT_READ_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_SECOND
#define CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_USECOND
#define CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_CLIENT_MAX_TIMEOUT_MSECOND
#define CPPHTTPLIB_CLIENT_MAX_TIMEOUT_MSECOND 0
#endif

#ifndef CPPHTTPLIB_IDLE_INTERVAL_SECOND
#define CPPHTTPLIB_IDLE_INTERVAL_SECOND 0
#endif

#ifndef CPPHTTPLIB_IDLE_INTERVAL_USECOND
#ifdef _WIN32
#define CPPHTTPLIB_IDLE_INTERVAL_USECOND 1000
#else
#define CPPHTTPLIB_IDLE_INTERVAL_USECOND 0
#endif
#endif

#ifndef CPPHTTPLIB_REQUEST_URI_MAX_LENGTH
#define CPPHTTPLIB_REQUEST_URI_MAX_LENGTH 8192
#endif

#ifndef CPPHTTPLIB_HEADER_MAX_LENGTH
#define CPPHTTPLIB_HEADER_MAX_LENGTH 8192
#endif

#ifndef CPPHTTPLIB_REDIRECT_MAX_COUNT
#define CPPHTTPLIB_REDIRECT_MAX_COUNT 20
#endif

#ifndef CPPHTTPLIB_MULTIPART_FORM_DATA_FILE_MAX_COUNT
#define CPPHTTPLIB_MULTIPART_FORM_DATA_FILE_MAX_COUNT 1024
#endif

#ifndef CPPHTTPLIB_PAYLOAD_MAX_LENGTH
#define CPPHTTPLIB_PAYLOAD_MAX_LENGTH ((std::numeric_limits<size_t>::max)())
#endif

#ifndef CPPHTTPLIB_FORM_URL_ENCODED_PAYLOAD_MAX_LENGTH
#define CPPHTTPLIB_FORM_URL_ENCODED_PAYLOAD_MAX_LENGTH 8192
#endif

#ifndef CPPHTTPLIB_RANGE_MAX_COUNT
#define CPPHTTPLIB_RANGE_MAX_COUNT 1024
#endif

#ifndef CPPHTTPLIB_TCP_NODELAY
#define CPPHTTPLIB_TCP_NODELAY false
#endif

#ifndef CPPHTTPLIB_IPV6_V6ONLY
#define CPPHTTPLIB_IPV6_V6ONLY false
#endif

#ifndef CPPHTTPLIB_RECV_BUFSIZ
#define CPPHTTPLIB_RECV_BUFSIZ size_t(16384u)
#endif

#ifndef CPPHTTPLIB_COMPRESSION_BUFSIZ
#define CPPHTTPLIB_COMPRESSION_BUFSIZ size_t(16384u)
#endif

#ifndef CPPHTTPLIB_THREAD_POOL_COUNT
#define CPPHTTPLIB_THREAD_POOL_COUNT                                           \
  ((std::max)(8u, std::thread::hardware_concurrency() > 0                      \
                      ? std::thread::hardware_concurrency() - 1                \
                      : 0))
#endif

#ifndef CPPHTTPLIB_RECV_FLAGS
#define CPPHTTPLIB_RECV_FLAGS 0
#endif

#ifndef CPPHTTPLIB_SEND_FLAGS
#define CPPHTTPLIB_SEND_FLAGS 0
#endif

#ifndef CPPHTTPLIB_LISTEN_BACKLOG
#define CPPHTTPLIB_LISTEN_BACKLOG std::thread::hardware_concurrency() * 4
#endif

/*
 * Headers
 */

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif //_CRT_SECURE_NO_WARNINGS

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif //_CRT_NONSTDC_NO_DEPRECATE

#if defined(_MSC_VER)
#if _MSC_VER < 1900
#error Sorry, Visual Studio versions prior to 2015 are not supported
#endif

#pragma comment(lib, "ws2_32.lib")

#ifdef _WIN64
using ssize_t = __int64;
#else
using ssize_t = long;
#endif
#endif // _MSC_VER

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFREG) == S_IFREG)
#endif // S_ISREG

#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFDIR) == S_IFDIR)
#endif // S_ISDIR

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#ifndef WSA_FLAG_NO_HANDLE_INHERIT
#define WSA_FLAG_NO_HANDLE_INHERIT 0x80
#endif

using socket_t = SOCKET;
#ifdef CPPHTTPLIB_USE_POLL
#define poll(fds, nfds, timeout) WSAPoll(fds, nfds, timeout)
#endif

#else // not _WIN32

#include <arpa/inet.h>
#if !defined(_AIX) && !defined(__MVS__)
#include <ifaddrs.h>
#endif
#ifdef __MVS__
#include <strings.h>
#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif
#endif
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#ifdef __linux__
#include <resolv.h>
#endif
#include <netinet/tcp.h>
#ifdef CPPHTTPLIB_USE_POLL
#include <poll.h>
#endif
#include <csignal>
#include <pthread.h>
#include <sys/mman.h>
#ifndef __VMS
#include <sys/select.h>
#endif
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using socket_t = int;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#endif //_WIN32

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <climits>
#include <condition_variable>
#include <cstring>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <re2/re2.h>

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
#ifdef _WIN32
#include <wincrypt.h>

// these are defined in wincrypt.h and it breaks compilation if BoringSSL is
// used
#undef X509_NAME
#undef X509_CERT_PAIR
#undef X509_EXTENSIONS
#undef PKCS7_SIGNER_INFO

#ifdef _MSC_VER
#pragma comment(lib, "crypt32.lib")
#endif
#elif defined(CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN) && defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_OSX
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#endif // TARGET_OS_OSX
#endif // _WIN32

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#if defined(_WIN32) && defined(OPENSSL_USE_APPLINK)
#include <openssl/applink.c>
#endif

#include <iostream>
#include <sstream>

#if defined(OPENSSL_IS_BORINGSSL) || defined(LIBRESSL_VERSION_NUMBER)
#if OPENSSL_VERSION_NUMBER < 0x1010107f
#error Please use OpenSSL or a current version of BoringSSL
#endif
#define SSL_get1_peer_certificate SSL_get_peer_certificate
#elif OPENSSL_VERSION_NUMBER < 0x30000000L
#error Sorry, OpenSSL versions prior to 3.0.0 are not supported
#endif

#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
#include <zlib.h>
#endif

#ifdef CPPHTTPLIB_BROTLI_SUPPORT
#include <brotli/decode.h>
#include <brotli/encode.h>
#endif

#include "../IOCPSocket.h"

#undef min
#undef max

/*
 * Declaration
 */
namespace httplib { namespace detail {
        /*
         * Backport std::make_unique from C++14.
         *
         * NOTE: This code came up with the following stackoverflow post:
         * https://stackoverflow.com/questions/10149840/c-arrays-and-make-unique
         *
         */

        template <class T, class... Args>
        auto make_unique(Args&&... args) -> std::unique_ptr<T> requires (!std::is_array_v<T>) {
            return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
        }

        template <class T>
        auto make_unique(const std::size_t n) -> std::unique_ptr<T> requires (std::is_array_v<T>) {
            using RT = std::remove_extent_t<T>;
            return std::unique_ptr<T>(new RT[n]);
        }

        namespace case_ignore {
            inline auto to_lower(const int c) -> unsigned char {
                const static unsigned char table[256] = {
                    0,
                    1,
                    2,
                    3,
                    4,
                    5,
                    6,
                    7,
                    8,
                    9,
                    10,
                    11,
                    12,
                    13,
                    14,
                    15,
                    16,
                    17,
                    18,
                    19,
                    20,
                    21,
                    22,
                    23,
                    24,
                    25,
                    26,
                    27,
                    28,
                    29,
                    30,
                    31,
                    32,
                    33,
                    34,
                    35,
                    36,
                    37,
                    38,
                    39,
                    40,
                    41,
                    42,
                    43,
                    44,
                    45,
                    46,
                    47,
                    48,
                    49,
                    50,
                    51,
                    52,
                    53,
                    54,
                    55,
                    56,
                    57,
                    58,
                    59,
                    60,
                    61,
                    62,
                    63,
                    64,
                    97,
                    98,
                    99,
                    100,
                    101,
                    102,
                    103,
                    104,
                    105,
                    106,
                    107,
                    108,
                    109,
                    110,
                    111,
                    112,
                    113,
                    114,
                    115,
                    116,
                    117,
                    118,
                    119,
                    120,
                    121,
                    122,
                    91,
                    92,
                    93,
                    94,
                    95,
                    96,
                    97,
                    98,
                    99,
                    100,
                    101,
                    102,
                    103,
                    104,
                    105,
                    106,
                    107,
                    108,
                    109,
                    110,
                    111,
                    112,
                    113,
                    114,
                    115,
                    116,
                    117,
                    118,
                    119,
                    120,
                    121,
                    122,
                    123,
                    124,
                    125,
                    126,
                    127,
                    128,
                    129,
                    130,
                    131,
                    132,
                    133,
                    134,
                    135,
                    136,
                    137,
                    138,
                    139,
                    140,
                    141,
                    142,
                    143,
                    144,
                    145,
                    146,
                    147,
                    148,
                    149,
                    150,
                    151,
                    152,
                    153,
                    154,
                    155,
                    156,
                    157,
                    158,
                    159,
                    160,
                    161,
                    162,
                    163,
                    164,
                    165,
                    166,
                    167,
                    168,
                    169,
                    170,
                    171,
                    172,
                    173,
                    174,
                    175,
                    176,
                    177,
                    178,
                    179,
                    180,
                    181,
                    182,
                    183,
                    184,
                    185,
                    186,
                    187,
                    188,
                    189,
                    190,
                    191,
                    224,
                    225,
                    226,
                    227,
                    228,
                    229,
                    230,
                    231,
                    232,
                    233,
                    234,
                    235,
                    236,
                    237,
                    238,
                    239,
                    240,
                    241,
                    242,
                    243,
                    244,
                    245,
                    246,
                    215,
                    248,
                    249,
                    250,
                    251,
                    252,
                    253,
                    254,
                    223,
                    224,
                    225,
                    226,
                    227,
                    228,
                    229,
                    230,
                    231,
                    232,
                    233,
                    234,
                    235,
                    236,
                    237,
                    238,
                    239,
                    240,
                    241,
                    242,
                    243,
                    244,
                    245,
                    246,
                    247,
                    248,
                    249,
                    250,
                    251,
                    252,
                    253,
                    254,
                    255,
                };
                return table[static_cast<unsigned char>(static_cast<char>(c))];
            }

            inline auto equal(const std::string& a, const std::string& b) -> bool {
                return a.size() == b.size() && std::equal(a.begin(),
                                                          a.end(),
                                                          b.begin(),
                                                          [](const char ca, const char cb) { return to_lower(ca) == to_lower(cb); });
            }

            struct equal_to {
                auto operator()(const std::string& a, const std::string& b) const -> bool { return equal(a, b); }
            };

            struct hash {
                auto operator()(const std::string& key) const -> size_t { return hash_core(key.data(), key.size(), 0); }

                static auto hash_core(const char* s, const size_t l, const size_t h) -> size_t {
                    return l == 0
                               ? h
                               : hash_core(s + 1,
                                           l - 1,
                                           // Unsets the 6 high bits of h, therefore no
                                           // overflow happens
                                           (std::numeric_limits<size_t>::max)() >> 6 & h * 33 ^ to_lower(*s));
                }
            };
        } // namespace case_ignore

        // This is based on
        // "http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4189".

        struct scope_exit {
            explicit scope_exit(std::function<void(void)>&& f) : exit_function(std::move(f)),
                                                                 execute_on_destruction{
                                                                     true
                                                                 } {}

            scope_exit(scope_exit&& rhs) noexcept : exit_function(std::move(rhs.exit_function)),
                                                    execute_on_destruction{
                                                        rhs.execute_on_destruction
                                                    } { rhs.release(); }

            ~scope_exit() {
                if (execute_on_destruction) {
                    this->exit_function();
                }
            }

            auto release() -> void { this->execute_on_destruction = false; }

        private:
            scope_exit(const scope_exit&) = delete;
            auto operator=(const scope_exit&) -> void = delete;
            auto operator=(scope_exit&&) -> scope_exit& = delete;

            std::function<void(void)> exit_function;
            bool execute_on_destruction;
        };
    } // namespace detail

    enum SSLVerifierResponse {
        // no decision has been made, use the built-in certificate verifier
        NoDecisionMade,
        // connection certificate is verified and accepted
        CertificateAccepted,
        // connection certificate was processed but is rejected
        CertificateRejected
    };

    enum StatusCode {
        // Information responses
        Continue_100          = 100,
        SwitchingProtocol_101 = 101,
        Processing_102        = 102,
        EarlyHints_103        = 103,

        // Successful responses
        OK_200                          = 200,
        Created_201                     = 201,
        Accepted_202                    = 202,
        NonAuthoritativeInformation_203 = 203,
        NoContent_204                   = 204,
        ResetContent_205                = 205,
        PartialContent_206              = 206,
        MultiStatus_207                 = 207,
        AlreadyReported_208             = 208,
        IMUsed_226                      = 226,

        // Redirection messages
        MultipleChoices_300   = 300,
        MovedPermanently_301  = 301,
        Found_302             = 302,
        SeeOther_303          = 303,
        NotModified_304       = 304,
        UseProxy_305          = 305,
        unused_306            = 306,
        TemporaryRedirect_307 = 307,
        PermanentRedirect_308 = 308,

        // Client error responses
        BadRequest_400                  = 400,
        Unauthorized_401                = 401,
        PaymentRequired_402             = 402,
        Forbidden_403                   = 403,
        NotFound_404                    = 404,
        MethodNotAllowed_405            = 405,
        NotAcceptable_406               = 406,
        ProxyAuthenticationRequired_407 = 407,
        RequestTimeout_408              = 408,
        Conflict_409                    = 409,
        Gone_410                        = 410,
        LengthRequired_411              = 411,
        PreconditionFailed_412          = 412,
        PayloadTooLarge_413             = 413,
        UriTooLong_414                  = 414,
        UnsupportedMediaType_415        = 415,
        RangeNotSatisfiable_416         = 416,
        ExpectationFailed_417           = 417,
        ImATeapot_418                   = 418,
        MisdirectedRequest_421          = 421,
        UnprocessableContent_422        = 422,
        Locked_423                      = 423,
        FailedDependency_424            = 424,
        TooEarly_425                    = 425,
        UpgradeRequired_426             = 426,
        PreconditionRequired_428        = 428,
        TooManyRequests_429             = 429,
        RequestHeaderFieldsTooLarge_431 = 431,
        UnavailableForLegalReasons_451  = 451,

        // Server error responses
        InternalServerError_500           = 500,
        NotImplemented_501                = 501,
        BadGateway_502                    = 502,
        ServiceUnavailable_503            = 503,
        GatewayTimeout_504                = 504,
        HttpVersionNotSupported_505       = 505,
        VariantAlsoNegotiates_506         = 506,
        InsufficientStorage_507           = 507,
        LoopDetected_508                  = 508,
        NotExtended_510                   = 510,
        NetworkAuthenticationRequired_511 = 511,
    };

    using Headers = std::unordered_multimap<std::string, std::string, detail::case_ignore::hash, detail::case_ignore::equal_to>;

    using Params = std::multimap<std::string, std::string>;
    using Match = std::vector<re2::StringPiece>;

    using Progress = std::function<bool(uint64_t current, uint64_t total)>;

    struct Response;
    using ResponseHandler = std::function<bool(const Response& response)>;

    struct MultipartFormData {
        std::string name;
        std::string content;
        std::string filename;
        std::string content_type;
    };

    using MultipartFormDataItems = std::vector<MultipartFormData>;
    using MultipartFormDataMap = std::multimap<std::string, MultipartFormData>;

    class DataSink {
    public:
        DataSink() : os(&sb_), sb_(*this) {}

        DataSink(const DataSink&) = delete;
        auto operator=(const DataSink&) -> DataSink& = delete;
        DataSink(DataSink&&) = delete;
        auto operator=(DataSink&&) -> DataSink& = delete;

        std::function<bool(const char* data, size_t data_len)> write;
        std::function<bool()> is_writable;
        std::function<void()> done;
        std::function<void(const Headers& trailer)> done_with_trailer;
        std::ostream os;

    private:
        class data_sink_streambuf final : public std::streambuf {
        public:
            explicit data_sink_streambuf(DataSink& sink) : sink_(sink) {}

        protected:
            auto xsputn(const char* s, const std::streamsize n) -> std::streamsize override {
                sink_.write(s, static_cast<size_t>(n));
                return n;
            }

        private:
            DataSink& sink_;
        };

        data_sink_streambuf sb_;
    };

    using ContentProvider = std::function<bool(size_t offset, size_t length, DataSink& sink)>;

    using ContentProviderWithoutLength = std::function<bool(size_t offset, DataSink& sink)>;

    using ContentProviderResourceReleaser = std::function<void(bool success)>;

    struct MultipartFormDataProvider {
        std::string name;
        ContentProviderWithoutLength provider;
        std::string filename;
        std::string content_type;
    };

    using MultipartFormDataProviderItems = std::vector<MultipartFormDataProvider>;

    using ContentReceiverWithProgress = std::function<bool(const char* data, size_t data_length, uint64_t offset, uint64_t total_length)>;

    using ContentReceiver = std::function<bool(const char* data, size_t data_length)>;

    using MultipartContentHeader = std::function<bool(const MultipartFormData& file)>;

    class ContentReader {
    public:
        using Reader = std::function<bool(ContentReceiver receiver)>;
        using MultipartReader = std::function<bool(MultipartContentHeader header, ContentReceiver receiver)>;

        ContentReader(Reader reader, MultipartReader multipart_reader) : reader_(std::move(reader)), multipart_reader_(std::move(multipart_reader)) {}

        auto operator()(MultipartContentHeader header, ContentReceiver receiver) const -> bool {
            return multipart_reader_(std::move(header), std::move(receiver));
        }

        auto operator()(ContentReceiver receiver) const -> bool { return reader_(std::move(receiver)); }

        Reader reader_;
        MultipartReader multipart_reader_;
    };

    using Range = std::pair<ssize_t, ssize_t>;
    using Ranges = std::vector<Range>;

    struct Request {
        std::string method;
        std::string path;
        Params params;
        Headers headers;
        std::string body;

        std::string remote_addr;
        int remote_port = -1;
        std::string local_addr;
        int local_port = -1;

        // for server
        std::string version;
        std::string target;
        MultipartFormDataMap files;
        Ranges ranges;
        Match matches;
        std::unordered_map<std::string, std::string> path_params;
        std::function<bool()> is_connection_closed = [] { return true; };

        // for client
        ResponseHandler response_handler;
        ContentReceiverWithProgress content_receiver;
        Progress progress;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        const SSL* ssl = nullptr;
#endif

        [[nodiscard]] auto has_header(const std::string& key) const -> bool;
        auto get_header_value(const std::string& key, const char* def = "", size_t id = 0) const -> std::string;
        [[nodiscard]] auto get_header_value_u64(const std::string& key, uint64_t def = 0, size_t id = 0) const -> uint64_t;
        [[nodiscard]] auto get_header_value_count(const std::string& key) const -> size_t;
        auto set_header(const std::string& key, const std::string& val) -> void;

        [[nodiscard]] auto has_param(const std::string& key) const -> bool;
        [[nodiscard]] auto get_param_value(const std::string& key, size_t id = 0) const -> std::string;
        [[nodiscard]] auto get_param_value_count(const std::string& key) const -> size_t;

        [[nodiscard]] auto is_multipart_form_data() const -> bool;

        [[nodiscard]] auto has_file(const std::string& key) const -> bool;
        [[nodiscard]] auto get_file_value(const std::string& key) const -> MultipartFormData;
        [[nodiscard]] auto get_file_values(const std::string& key) const -> std::vector<MultipartFormData>;

        // private members...
        size_t redirect_count_ = CPPHTTPLIB_REDIRECT_MAX_COUNT;
        size_t content_length_ = 0;
        ContentProvider content_provider_;
        bool is_chunked_content_provider_ = false;
        size_t authorization_count_ = 0;
        std::chrono::time_point<std::chrono::steady_clock> start_time_ = std::chrono::steady_clock::time_point::min();
    };

    struct Response {
        std::string version;
        int status = -1;
        std::string reason;
        Headers headers;
        std::string body;
        std::string location; // Redirect location

        [[nodiscard]] auto has_header(const std::string& key) const -> bool;
        auto get_header_value(const std::string& key, const char* def = "", size_t id = 0) const -> std::string;
        [[nodiscard]] auto get_header_value_u64(const std::string& key, uint64_t def = 0, size_t id = 0) const -> uint64_t;
        [[nodiscard]] auto get_header_value_count(const std::string& key) const -> size_t;
        auto set_header(const std::string& key, const std::string& val) -> void;

        auto set_redirect(const std::string& url, int status = Found_302) -> void;
        auto set_content(const char* s, size_t n, const std::string& content_type) -> void;
        auto set_content(const std::string& s, const std::string& content_type) -> void;
        auto set_content(std::string&& s, const std::string& content_type) -> void;

        auto set_content_provider(size_t length,
                                  const std::string& content_type,
                                  ContentProvider provider,
                                  ContentProviderResourceReleaser resource_releaser = nullptr) -> void;

        auto set_content_provider(const std::string& content_type,
                                  ContentProviderWithoutLength provider,
                                  ContentProviderResourceReleaser resource_releaser = nullptr) -> void;

        auto set_chunked_content_provider(const std::string& content_type,
                                          ContentProviderWithoutLength provider,
                                          ContentProviderResourceReleaser resource_releaser = nullptr) -> void;

        auto set_file_content(const std::string& path, const std::string& content_type) -> void;
        auto set_file_content(const std::string& path) -> void;

        Response() = default;
        Response(const Response&) = default;
        auto operator=(const Response&) -> Response& = default;
        Response(Response&&) = default;
        auto operator=(Response&&) -> Response& = default;

        ~Response() {
            if (content_provider_resource_releaser_) {
                content_provider_resource_releaser_(content_provider_success_);
            }
        }

        // private members...
        size_t content_length_ = 0;
        ContentProvider content_provider_;
        ContentProviderResourceReleaser content_provider_resource_releaser_;
        bool is_chunked_content_provider_ = false;
        bool content_provider_success_ = false;
        std::string file_content_path_;
        std::string file_content_content_type_;
    };

    class Stream {
    public:
        virtual ~Stream() = default;

        [[nodiscard]] virtual auto is_readable() const -> bool = 0;
        [[nodiscard]] virtual auto is_writable() const -> bool = 0;

        virtual auto read(char* ptr, size_t size) -> ssize_t = 0;
        virtual auto write(const char* ptr, size_t size) -> ssize_t = 0;
        virtual auto get_remote_ip_and_port(std::string& ip, int& port) const -> void = 0;
        virtual auto get_local_ip_and_port(std::string& ip, int& port) const -> void = 0;
        [[nodiscard]] virtual auto socket() const -> socket_t = 0;

        [[nodiscard]] virtual auto duration() const -> time_t = 0;

        auto write(const char* ptr) -> ssize_t;
        auto write(const std::string& s) -> ssize_t;
    };

    class TaskQueue {
    public:
        TaskQueue() = default;
        virtual ~TaskQueue() = default;

        virtual auto enqueue(std::function<void()> fn) -> bool = 0;
        virtual auto shutdown() -> void = 0;

        virtual auto on_idle() -> void {}
    };

    class ThreadPool final : public TaskQueue {
    public:
        explicit ThreadPool(size_t n, const size_t mqr = 0) : shutdown_(false), max_queued_requests_(mqr) {
            while (n) {
                threads_.emplace_back(worker(*this));
                n--;
            }
        }

        ThreadPool(const ThreadPool&) = delete;
        ~ThreadPool() override = default;

        auto enqueue(std::function<void()> fn) -> bool override {
            {
                std::unique_lock lock(mutex_);
                if (max_queued_requests_ > 0 && jobs_.size() >= max_queued_requests_) {
                    return false;
                }
                jobs_.push_back(std::move(fn));
            }

            cond_.notify_one();
            return true;
        }

        auto shutdown() -> void override {
            // Stop all worker threads...
            {
                std::unique_lock lock(mutex_);
                shutdown_ = true;
            }

            cond_.notify_all();

            // Join...
            for (auto& t : threads_) {
                t.join();
            }
        }

    private:
        struct worker {
            explicit worker(ThreadPool& pool) : pool_(pool) {}

            auto operator()() const -> void {
                for (;;) {
                    std::function<void()> fn;
                    {
                        std::unique_lock lock(pool_.mutex_);

                        pool_.cond_.wait(lock,
                                         [&] { return !pool_.jobs_.empty() || pool_.shutdown_; });

                        if (pool_.shutdown_ && pool_.jobs_.empty()) {
                            break;
                        }

                        fn = pool_.jobs_.front();
                        pool_.jobs_.pop_front();
                    }

                    assert(true == static_cast<bool>(fn));
                    fn();
                }

#if defined(CPPHTTPLIB_OPENSSL_SUPPORT) && !defined(OPENSSL_IS_BORINGSSL) &&   \
    !defined(LIBRESSL_VERSION_NUMBER)
                OPENSSL_thread_stop();
#endif
            }

            ThreadPool& pool_;
        };

        friend struct worker;

        std::vector<std::thread> threads_;
        std::list<std::function<void()>> jobs_;

        bool shutdown_;
        size_t max_queued_requests_ = 0;

        std::condition_variable cond_;
        std::mutex mutex_;
    };

    using Logger = std::function<void(const Request&, const Response&)>;

    using SocketOptions = std::function<void(socket_t sock)>;

    auto default_socket_options(socket_t sock) -> void;

    auto status_message(int status) -> const char*;

    auto get_bearer_token_auth(const Request& req) -> std::string;

    namespace detail {
        class MatcherBase {
        public:
            virtual ~MatcherBase() = default;

            // Match request path and populate its matches and
            virtual auto match(Request& request) const -> bool = 0;
        };

        /**
         * Captures parameters in request path and stores them in Request::path_params
         *
         * Capture name is a substring of a pattern from : to /.
         * The rest of the pattern is matched agains the request path directly
         * Parameters are captured starting from the next character after
         * the end of the last matched static pattern fragment until the next /.
         *
         * Example pattern:
         * "/path/fragments/:capture/more/fragments/:second_capture"
         * Static fragments:
         * "/path/fragments/", "more/fragments/"
         *
         * Given the following request path:
         * "/path/fragments/:1/more/fragments/:2"
         * the resulting capture will be
         * {{"capture", "1"}, {"second_capture", "2"}}
         */
        class PathParamsMatcher final : public MatcherBase {
        public:
            PathParamsMatcher(const std::string& pattern);

            auto match(Request& request) const -> bool override;

        private:
            // Treat segment separators as the end of path parameter capture
            // Does not need to handle query parameters as they are parsed before path
            // matching
            static constexpr char separator = '/';

            // Contains static path fragments to match against, excluding the '/' after
            // path params
            // Fragments are separated by path params
            std::vector<std::string> static_fragments_;
            // Stores the names of the path parameters to be used as keys in the
            // Request::path_params map
            std::vector<std::string> param_names_;

            size_t static_total_length_;
        };

        /**
         * Performs std::regex_match on request path
         * and stores the result in Request::matches
         *
         * Note that regex match is performed directly on the whole request.
         * This means that wildcard patterns may match multiple path segments with /:
         * "/begin/(.*)/end" will match both "/begin/middle/end" and "/begin/1/2/end".
         */
        class RegexMatcher final : public MatcherBase {
        public:
            RegexMatcher(const std::string& pattern) : regex_(pattern) {
                size_ = regex_.NumberOfCapturingGroups() + 1;
            }

            auto match(Request& request) const -> bool override;

        private:
            int size_;
            re2::RE2 regex_;
        };

        auto write_headers(Stream& strm, const Headers& headers) -> ssize_t;
    } // namespace detail

    class Server {
    public:
        using Handler = std::function<void(const Request&, Response&)>;

        using ExceptionHandler = std::function<void(const Request&, Response&, std::exception_ptr ep)>;

        enum class HandlerResponse {
            Handled,
            Unhandled,
        };

        using HandlerWithResponse = std::function<HandlerResponse(const Request&, Response&)>;

        using HandlerWithContentReader = std::function<void(const Request&, Response&, const ContentReader& content_reader)>;

        using Expect100ContinueHandler = std::function<int(const Request&, Response&)>;

        Server();

        virtual ~Server();

        virtual auto is_valid() const -> bool;

        auto Get(const std::string& pattern, Handler handler) -> Server&;
        auto Post(const std::string& pattern, Handler handler) -> Server&;
        auto Post(const std::string& pattern, HandlerWithContentReader handler) -> Server&;
        auto Put(const std::string& pattern, Handler handler) -> Server&;
        auto Put(const std::string& pattern, HandlerWithContentReader handler) -> Server&;
        auto Patch(const std::string& pattern, Handler handler) -> Server&;
        auto Patch(const std::string& pattern, HandlerWithContentReader handler) -> Server&;
        auto Delete(const std::string& pattern, Handler handler) -> Server&;
        auto Delete(const std::string& pattern, HandlerWithContentReader handler) -> Server&;
        auto Options(const std::string& pattern, Handler handler) -> Server&;

        auto set_base_dir(const std::string& dir, const std::string& mount_point = std::string()) -> bool;
        auto set_mount_point(const std::string& mount_point, const std::string& dir, Headers headers = Headers()) -> bool;
        auto remove_mount_point(const std::string& mount_point) -> bool;
        auto set_file_extension_and_mimetype_mapping(const std::string& ext, const std::string& mime) -> Server&;
        auto set_default_file_mimetype(const std::string& mime) -> Server&;
        auto set_file_request_handler(Handler handler) -> Server&;

        template <class ErrorHandlerFunc>
        auto set_error_handler(ErrorHandlerFunc&& handler) -> Server& {
            return set_error_handler_core(std::forward<ErrorHandlerFunc>(handler), std::is_convertible<ErrorHandlerFunc, HandlerWithResponse>{});
        }

        auto set_exception_handler(ExceptionHandler handler) -> Server&;
        auto set_pre_routing_handler(HandlerWithResponse handler) -> Server&;
        auto set_post_routing_handler(Handler handler) -> Server&;

        auto set_expect_100_continue_handler(Expect100ContinueHandler handler) -> Server&;
        auto set_logger(Logger logger) -> Server&;

        auto set_address_family(int family) -> Server&;
        auto set_tcp_nodelay(bool on) -> Server&;
        auto set_ipv6_v6only(bool on) -> Server&;
        auto set_socket_options(SocketOptions socket_options) -> Server&;

        auto set_default_headers(Headers headers) -> Server&;
        auto set_header_writer(const std::function<ssize_t(Stream&, Headers&)>& writer) -> Server&;

        auto set_keep_alive_max_count(size_t count) -> Server&;
        auto set_keep_alive_timeout(time_t sec) -> Server&;

        auto set_read_timeout(time_t sec, time_t usec = 0) -> Server&;
        template <class Rep, class Period>
        auto set_read_timeout(const std::chrono::duration<Rep, Period>& duration) -> Server&;

        auto set_write_timeout(time_t sec, time_t usec = 0) -> Server&;
        template <class Rep, class Period>
        auto set_write_timeout(const std::chrono::duration<Rep, Period>& duration) -> Server&;

        auto set_idle_interval(time_t sec, time_t usec = 0) -> Server&;
        template <class Rep, class Period>
        auto set_idle_interval(const std::chrono::duration<Rep, Period>& duration) -> Server&;

        auto set_payload_max_length(size_t length) -> Server&;

        auto bind_to_port(const std::string& host, int port, int socket_flags = 0) -> bool;
        auto bind_to_any_port(const std::string& host, int socket_flags = 0) -> int;
        auto listen_after_bind() -> bool;

        auto listen(const std::string& host, int port, int socket_flags = 0) -> bool;

        auto is_running() const -> bool;
        auto wait_until_ready() const -> void;
        auto stop() -> void;
        auto decommission() -> void;

        std::function<TaskQueue *(void)> new_task_queue;

    protected:
        auto process_request(Stream& strm,
                             const std::string& remote_addr,
                             int remote_port,
                             const std::string& local_addr,
                             int local_port,
                             bool close_connection,
                             bool& connection_closed,
                             const std::function<void(Request&)>& setup_request) const -> bool;

        std::atomic<socket_t> svr_sock_{
            INVALID_SOCKET
        };
        size_t keep_alive_max_count_ = CPPHTTPLIB_KEEPALIVE_MAX_COUNT;
        time_t keep_alive_timeout_sec_ = CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND;
        time_t read_timeout_sec_ = CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND;
        time_t read_timeout_usec_ = CPPHTTPLIB_SERVER_READ_TIMEOUT_USECOND;
        time_t write_timeout_sec_ = CPPHTTPLIB_SERVER_WRITE_TIMEOUT_SECOND;
        time_t write_timeout_usec_ = CPPHTTPLIB_SERVER_WRITE_TIMEOUT_USECOND;
        time_t idle_interval_sec_ = CPPHTTPLIB_IDLE_INTERVAL_SECOND;
        time_t idle_interval_usec_ = CPPHTTPLIB_IDLE_INTERVAL_USECOND;
        size_t payload_max_length_ = CPPHTTPLIB_PAYLOAD_MAX_LENGTH;

    private:
        using Handlers = std::vector<std::pair<std::unique_ptr<detail::MatcherBase>, Handler>>;
        using HandlersForContentReader = std::vector<std::pair<std::unique_ptr<detail::MatcherBase>, HandlerWithContentReader>>;

        static auto make_matcher(const std::string& pattern) -> std::unique_ptr<detail::MatcherBase>;

        auto set_error_handler_core(HandlerWithResponse handler, std::true_type) -> Server&;
        auto set_error_handler_core(const Handler& handler, std::false_type) -> Server&;

        auto create_server_socket(const std::string& host, int port, int socket_flags, const SocketOptions& socket_options) const -> socket_t;
        auto bind_internal(const std::string& host, int port, int socket_flags) -> int;
        auto listen_internal() -> bool;

        auto routing(Request& req, Response& res, Stream& strm) const -> bool;
        auto handle_file_request(const Request& req, Response& res, bool head = false) const -> bool;
        static auto dispatch_request(Request& req, Response& res, const Handlers& handlers) -> bool;
        static auto dispatch_request_for_content_reader(Request& req,
                                                        Response& res,
                                                        const ContentReader& content_reader,
                                                        const HandlersForContentReader& handlers) -> bool;

        static auto parse_request_line(const char* s, Request& req) -> bool;
        static auto apply_ranges(const Request& req, Response& res, std::string& content_type, std::string& boundary) -> void;
        auto write_response(Stream& strm, bool close_connection, Request& req, Response& res) const -> bool;
        auto write_response_with_content(Stream& strm, bool close_connection, const Request& req, Response& res) const -> bool;
        auto write_response_core(Stream& strm, bool close_connection, const Request& req, Response& res, bool need_apply_ranges) const -> bool;
        auto write_content_with_provider(Stream& strm,
                                         const Request& req,
                                         Response& res,
                                         const std::string& boundary,
                                         const std::string& content_type) const -> bool;
        auto read_content(Stream& strm, Request& req, Response& res) const -> bool;
        auto read_content_with_content_receiver(Stream& strm,
                                                Request& req,
                                                Response& res,
                                                const ContentReceiver& receiver,
                                                const MultipartContentHeader& multipart_header,
                                                const ContentReceiver& multipart_receiver) const -> bool;
        auto read_content_core(Stream& strm,
                               Request& req,
                               Response& res,
                               const ContentReceiver& receiver,
                               const MultipartContentHeader& multipart_header,
                               const ContentReceiver& multipart_receiver) const -> bool;

        virtual auto process_and_close_socket(socket_t sock, const std::string& ip, int port) -> bool;

        std::atomic<bool> is_running_{
            false
        };
        std::atomic<bool> is_decommisioned{
            false
        };

        struct MountPointEntry {
            std::string mount_point;
            std::string base_dir;
            Headers headers;
        };

        std::vector<MountPointEntry> base_dirs_;
        std::map<std::string, std::string> file_extension_and_mimetype_map_;
        std::string default_file_mimetype_ = "application/octet-stream";
        Handler file_request_handler_;

        Handlers get_handlers_;
        Handlers post_handlers_;
        HandlersForContentReader post_handlers_for_content_reader_;
        Handlers put_handlers_;
        HandlersForContentReader put_handlers_for_content_reader_;
        Handlers patch_handlers_;
        HandlersForContentReader patch_handlers_for_content_reader_;
        Handlers delete_handlers_;
        HandlersForContentReader delete_handlers_for_content_reader_;
        Handlers options_handlers_;

        HandlerWithResponse error_handler_;
        ExceptionHandler exception_handler_;
        HandlerWithResponse pre_routing_handler_;
        Handler post_routing_handler_;
        Expect100ContinueHandler expect_100_continue_handler_;

        Logger logger_;

        int address_family_ = AF_UNSPEC;
        bool tcp_nodelay_ = CPPHTTPLIB_TCP_NODELAY;
        bool ipv6_v6only_ = CPPHTTPLIB_IPV6_V6ONLY;
        SocketOptions socket_options_ = default_socket_options;

        Headers default_headers_;
        std::function<ssize_t(Stream&, Headers&)> header_writer_ = detail::write_headers;
    };

    enum class Error {
        Success = 0,
        Unknown,
        Connection,
        BindIPAddress,
        Read,
        Write,
        ExceedRedirectCount,
        Canceled,
        SSLConnection,
        SSLLoadingCerts,
        SSLServerVerification,
        SSLServerHostnameVerification,
        UnsupportedMultipartBoundaryChars,
        Compression,
        ConnectionTimeout,
        ProxyConnection,

        // For internal use only
        SSLPeerCouldBeClosed_,
    };

    auto to_string(Error error) -> std::string;

    auto operator<<(std::ostream& os, const Error& obj) -> std::ostream&;

    class Result {
    public:
        Result() = default;

        Result(std::unique_ptr<Response>&& res, const Error err, Headers&& request_headers = Headers{}) : res_(std::move(res)),
            err_(err),
            request_headers_(std::move(request_headers)) {}

        // Response
        operator bool() const { return res_ != nullptr; }

        auto operator==(std::nullptr_t) const -> bool { return res_ == nullptr; }

        auto operator!=(std::nullptr_t) const -> bool { return res_ != nullptr; }

        [[nodiscard]] auto value() const -> const Response& { return *res_; }

        auto value() -> Response& { return *res_; }

        auto operator*() const -> const Response& { return *res_; }

        auto operator*() -> Response& { return *res_; }

        auto operator->() const -> const Response* { return res_.get(); }

        auto operator->() -> Response* { return res_.get(); }

        // Error
        [[nodiscard]] auto error() const -> Error { return err_; }

        // Request Headers
        [[nodiscard]] auto has_request_header(const std::string& key) const -> bool;
        auto get_request_header_value(const std::string& key, const char* def = "", size_t id = 0) const -> std::string;
        [[nodiscard]] auto get_request_header_value_u64(const std::string& key, uint64_t def = 0, size_t id = 0) const -> uint64_t;
        [[nodiscard]] auto get_request_header_value_count(const std::string& key) const -> size_t;

    private:
        std::unique_ptr<Response> res_;
        Error err_ = Error::Unknown;
        Headers request_headers_;
    };

    class ClientImpl {
    public:
        explicit ClientImpl(const std::string& host);

        explicit ClientImpl(const std::string& host, int port);

        explicit ClientImpl(const std::string& host, int port, std::string client_cert_path, std::string client_key_path);

        virtual ~ClientImpl();

        virtual auto is_valid() const -> bool;

        auto Get(const std::string& path) -> Result;
        auto Get(const std::string& path, const Headers& headers) -> Result;
        auto Get(const std::string& path, Progress progress) -> Result;
        auto Get(const std::string& path, const Headers& headers, Progress progress) -> Result;
        auto Get(const std::string& path, const ContentReceiver& content_receiver) -> Result;
        auto Get(const std::string& path, const Headers& headers, const ContentReceiver& content_receiver) -> Result;
        auto Get(const std::string& path, const ContentReceiver& content_receiver, Progress progress) -> Result;
        auto Get(const std::string& path, const Headers& headers, const ContentReceiver& content_receiver, Progress progress) -> Result;
        auto Get(const std::string& path, ResponseHandler response_handler, const ContentReceiver& content_receiver) -> Result;
        auto Get(const std::string& path,
                 const Headers& headers,
                 ResponseHandler response_handler,
                 const ContentReceiver& content_receiver) -> Result;
        auto Get(const std::string& path, ResponseHandler response_handler, const ContentReceiver& content_receiver, Progress progress) -> Result;
        auto Get(const std::string& path,
                 const Headers& headers,
                 ResponseHandler response_handler,
                 const ContentReceiver& content_receiver,
                 Progress progress) -> Result;

        auto Get(const std::string& path, const Params& params, const Headers& headers, Progress progress = nullptr) -> Result;
        auto Get(const std::string& path,
                 const Params& params,
                 const Headers& headers,
                 const ContentReceiver& content_receiver,
                 Progress progress = nullptr) -> Result;
        auto Get(const std::string& path,
                 const Params& params,
                 const Headers& headers,
                 ResponseHandler response_handler,
                 const ContentReceiver& content_receiver,
                 Progress progress = nullptr) -> Result;

        auto Head(const std::string& path) -> Result;
        auto Head(const std::string& path, const Headers& headers) -> Result;

        auto Post(const std::string& path) -> Result;
        auto Post(const std::string& path, const Headers& headers) -> Result;
        auto Post(const std::string& path, const char* body, size_t content_length, const std::string& content_type) -> Result;
        auto Post(const std::string& path,
                  const Headers& headers,
                  const char* body,
                  size_t content_length,
                  const std::string& content_type) -> Result;
        auto Post(const std::string& path,
                  const Headers& headers,
                  const char* body,
                  size_t content_length,
                  const std::string& content_type,
                  const Progress& progress) -> Result;
        auto Post(const std::string& path, const std::string& body, const std::string& content_type) -> Result;
        auto Post(const std::string& path, const std::string& body, const std::string& content_type, const Progress& progress) -> Result;
        auto Post(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) -> Result;
        auto Post(const std::string& path,
                  const Headers& headers,
                  const std::string& body,
                  const std::string& content_type,
                  const Progress& progress) -> Result;
        auto Post(const std::string& path, size_t content_length, ContentProvider content_provider, const std::string& content_type) -> Result;
        auto Post(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) -> Result;
        auto Post(const std::string& path,
                  const Headers& headers,
                  size_t content_length,
                  ContentProvider content_provider,
                  const std::string& content_type) -> Result;
        auto Post(const std::string& path,
                  const Headers& headers,
                  ContentProviderWithoutLength content_provider,
                  const std::string& content_type) -> Result;
        auto Post(const std::string& path, const Params& params) -> Result;
        auto Post(const std::string& path, const Headers& headers, const Params& params) -> Result;
        auto Post(const std::string& path, const Headers& headers, const Params& params, const Progress& progress) -> Result;
        auto Post(const std::string& path, const MultipartFormDataItems& items) -> Result;
        auto Post(const std::string& path, const Headers& headers, const MultipartFormDataItems& items) -> Result;
        auto Post(const std::string& path, const Headers& headers, const MultipartFormDataItems& items, const std::string& boundary) -> Result;
        auto Post(const std::string& path,
                  const Headers& headers,
                  const MultipartFormDataItems& items,
                  const MultipartFormDataProviderItems& provider_items) -> Result;

        auto Put(const std::string& path) -> Result;
        auto Put(const std::string& path, const char* body, size_t content_length, const std::string& content_type) -> Result;
        auto Put(const std::string& path, const Headers& headers, const char* body, size_t content_length, const std::string& content_type) -> Result;
        auto Put(const std::string& path,
                 const Headers& headers,
                 const char* body,
                 size_t content_length,
                 const std::string& content_type,
                 const Progress& progress) -> Result;
        auto Put(const std::string& path, const std::string& body, const std::string& content_type) -> Result;
        auto Put(const std::string& path, const std::string& body, const std::string& content_type, const Progress& progress) -> Result;
        auto Put(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) -> Result;
        auto Put(const std::string& path,
                 const Headers& headers,
                 const std::string& body,
                 const std::string& content_type,
                 const Progress& progress) -> Result;
        auto Put(const std::string& path, size_t content_length, ContentProvider content_provider, const std::string& content_type) -> Result;
        auto Put(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) -> Result;
        auto Put(const std::string& path,
                 const Headers& headers,
                 size_t content_length,
                 ContentProvider content_provider,
                 const std::string& content_type) -> Result;
        auto Put(const std::string& path,
                 const Headers& headers,
                 ContentProviderWithoutLength content_provider,
                 const std::string& content_type) -> Result;
        auto Put(const std::string& path, const Params& params) -> Result;
        auto Put(const std::string& path, const Headers& headers, const Params& params) -> Result;
        auto Put(const std::string& path, const Headers& headers, const Params& params, const Progress& progress) -> Result;
        auto Put(const std::string& path, const MultipartFormDataItems& items) -> Result;
        auto Put(const std::string& path, const Headers& headers, const MultipartFormDataItems& items) -> Result;
        auto Put(const std::string& path, const Headers& headers, const MultipartFormDataItems& items, const std::string& boundary) -> Result;
        auto Put(const std::string& path,
                 const Headers& headers,
                 const MultipartFormDataItems& items,
                 const MultipartFormDataProviderItems& provider_items) -> Result;

        auto Patch(const std::string& path) -> Result;
        auto Patch(const std::string& path, const char* body, size_t content_length, const std::string& content_type) -> Result;
        auto Patch(const std::string& path,
                   const char* body,
                   size_t content_length,
                   const std::string& content_type,
                   const Progress& progress) -> Result;
        auto Patch(const std::string& path,
                   const Headers& headers,
                   const char* body,
                   size_t content_length,
                   const std::string& content_type) -> Result;
        auto Patch(const std::string& path,
                   const Headers& headers,
                   const char* body,
                   size_t content_length,
                   const std::string& content_type,
                   const Progress& progress) -> Result;
        auto Patch(const std::string& path, const std::string& body, const std::string& content_type) -> Result;
        auto Patch(const std::string& path, const std::string& body, const std::string& content_type, const Progress& progress) -> Result;
        auto Patch(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) -> Result;
        auto Patch(const std::string& path,
                   const Headers& headers,
                   const std::string& body,
                   const std::string& content_type,
                   const Progress& progress) -> Result;
        auto Patch(const std::string& path, size_t content_length, ContentProvider content_provider, const std::string& content_type) -> Result;
        auto Patch(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) -> Result;
        auto Patch(const std::string& path,
                   const Headers& headers,
                   size_t content_length,
                   ContentProvider content_provider,
                   const std::string& content_type) -> Result;
        auto Patch(const std::string& path,
                   const Headers& headers,
                   ContentProviderWithoutLength content_provider,
                   const std::string& content_type) -> Result;

        auto Delete(const std::string& path) -> Result;
        auto Delete(const std::string& path, const Headers& headers) -> Result;
        auto Delete(const std::string& path, const char* body, size_t content_length, const std::string& content_type) -> Result;
        auto Delete(const std::string& path,
                    const char* body,
                    size_t content_length,
                    const std::string& content_type,
                    const Progress& progress) -> Result;
        auto Delete(const std::string& path,
                    const Headers& headers,
                    const char* body,
                    size_t content_length,
                    const std::string& content_type) -> Result;
        auto Delete(const std::string& path,
                    const Headers& headers,
                    const char* body,
                    size_t content_length,
                    const std::string& content_type,
                    const Progress& progress) -> Result;
        auto Delete(const std::string& path, const std::string& body, const std::string& content_type) -> Result;
        auto Delete(const std::string& path, const std::string& body, const std::string& content_type, const Progress& progress) -> Result;
        auto Delete(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) -> Result;
        auto Delete(const std::string& path,
                    const Headers& headers,
                    const std::string& body,
                    const std::string& content_type,
                    const Progress& progress) -> Result;

        auto Options(const std::string& path) -> Result;
        auto Options(const std::string& path, const Headers& headers) -> Result;

        auto send(Request& req, Response& res, Error& error) -> bool;
        auto send(const Request& req) -> Result;

        auto stop() -> void;

        auto host() const -> std::string;
        auto port() const -> int;

        auto is_socket_open() const -> size_t;
        auto socket() const -> socket_t;

        auto set_hostname_addr_map(std::map<std::string, std::string> addr_map) -> void;

        auto set_default_headers(Headers headers) -> void;

        auto set_header_writer(const std::function<ssize_t(Stream&, Headers&)>& writer) -> void;

        auto set_address_family(int family) -> void;
        auto set_tcp_nodelay(bool on) -> void;
        auto set_ipv6_v6only(bool on) -> void;
        auto set_socket_options(SocketOptions socket_options) -> void;

        auto set_connection_timeout(time_t sec, time_t usec = 0) -> void;
        template <class Rep, class Period>
        auto set_connection_timeout(const std::chrono::duration<Rep, Period>& duration) -> void;

        auto set_read_timeout(time_t sec, time_t usec = 0) -> void;
        template <class Rep, class Period>
        auto set_read_timeout(const std::chrono::duration<Rep, Period>& duration) -> void;

        auto set_write_timeout(time_t sec, time_t usec = 0) -> void;
        template <class Rep, class Period>
        auto set_write_timeout(const std::chrono::duration<Rep, Period>& duration) -> void;

        auto set_max_timeout(time_t msec) -> void;
        template <class Rep, class Period>
        auto set_max_timeout(const std::chrono::duration<Rep, Period>& duration) -> void;

        auto set_basic_auth(const std::string& username, const std::string& password) -> void;
        auto set_bearer_token_auth(const std::string& token) -> void;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto set_digest_auth(const std::string& username, const std::string& password) -> void;
#endif

        auto set_keep_alive(bool on) -> void;
        auto set_follow_location(bool on) -> void;

        auto set_url_encode(bool on) -> void;

        auto set_compress(bool on) -> void;

        auto set_decompress(bool on) -> void;

        auto set_interface(const std::string& intf) -> void;

        auto set_proxy(const std::string& host, int port) -> void;
        auto set_proxy_basic_auth(const std::string& username, const std::string& password) -> void;
        auto set_proxy_bearer_token_auth(const std::string& token) -> void;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto set_proxy_digest_auth(const std::string& username, const std::string& password) -> void;
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto set_ca_cert_path(const std::string& ca_cert_file_path, const std::string& ca_cert_dir_path = std::string()) -> void;
        auto set_ca_cert_store(X509_STORE* ca_cert_store) -> void;
        static auto create_ca_cert_store(const char* ca_cert, std::size_t size) -> X509_STORE*;
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto enable_server_certificate_verification(bool enabled) -> void;
        auto enable_server_hostname_verification(bool enabled) -> void;
        auto set_server_certificate_verifier(std::function<SSLVerifierResponse(SSL* ssl)> verifier) -> void;
#endif

        auto set_logger(Logger logger) -> void;

    protected:
        struct Socket {
            socket_t sock = INVALID_SOCKET;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
            SSL* ssl = nullptr;
#endif

            [[nodiscard]] auto is_open() const -> bool { return sock != INVALID_SOCKET; }
        };

        virtual auto create_and_connect_socket(Socket& socket, Error& error) -> bool;

        // All of:
        //   shutdown_ssl
        //   shutdown_socket
        //   close_socket
        // should ONLY be called when socket_mutex_ is locked.
        // Also, shutdown_ssl and close_socket should also NOT be called concurrently
        // with a DIFFERENT thread sending requests using that socket.
        virtual auto shutdown_ssl(Socket& socket, bool shutdown_gracefully) -> void;
        static auto shutdown_socket(const Socket& socket) -> void;
        auto close_socket(Socket& socket) const -> void;

        auto process_request(Stream& strm, Request& req, Response& res, bool close_connection, Error& error) const -> bool;

        auto write_content_with_provider(Stream& strm, const Request& req, Error& error) const -> bool;

        auto copy_settings(const ClientImpl& rhs) -> void;

        // Socket endpoint information
        const std::string host_;
        const int port_;
        const std::string host_and_port_;

        // Current open socket
        Socket socket_;
        mutable std::mutex socket_mutex_;
        std::recursive_mutex request_mutex_;

        // These are all protected under socket_mutex
        size_t socket_requests_in_flight_ = 0;
        std::thread::id socket_requests_are_from_thread_ = std::thread::id();
        bool socket_should_be_closed_when_request_is_done_ = false;

        // Hostname-IP map
        std::map<std::string, std::string> addr_map_;

        // Default headers
        Headers default_headers_;

        // Header writer
        std::function<ssize_t(Stream&, Headers&)> header_writer_ = detail::write_headers;

        // Settings
        std::string client_cert_path_;
        std::string client_key_path_;

        time_t connection_timeout_sec_ = CPPHTTPLIB_CONNECTION_TIMEOUT_SECOND;
        time_t connection_timeout_usec_ = CPPHTTPLIB_CONNECTION_TIMEOUT_USECOND;
        time_t read_timeout_sec_ = CPPHTTPLIB_CLIENT_READ_TIMEOUT_SECOND;
        time_t read_timeout_usec_ = CPPHTTPLIB_CLIENT_READ_TIMEOUT_USECOND;
        time_t write_timeout_sec_ = CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_SECOND;
        time_t write_timeout_usec_ = CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_USECOND;
        time_t max_timeout_msec_ = CPPHTTPLIB_CLIENT_MAX_TIMEOUT_MSECOND;

        std::string basic_auth_username_;
        std::string basic_auth_password_;
        std::string bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        std::string digest_auth_username_;
        std::string digest_auth_password_;
#endif

        bool keep_alive_ = false;
        bool follow_location_ = false;

        bool url_encode_ = true;

        int address_family_ = AF_UNSPEC;
        bool tcp_nodelay_ = CPPHTTPLIB_TCP_NODELAY;
        bool ipv6_v6only_ = CPPHTTPLIB_IPV6_V6ONLY;
        SocketOptions socket_options_ = nullptr;

        bool compress_ = false;
        bool decompress_ = true;

        std::string interface_;

        std::string proxy_host_;
        int proxy_port_ = -1;

        std::string proxy_basic_auth_username_;
        std::string proxy_basic_auth_password_;
        std::string proxy_bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        std::string proxy_digest_auth_username_;
        std::string proxy_digest_auth_password_;
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        std::string ca_cert_file_path_;
        std::string ca_cert_dir_path_;

        X509_STORE* ca_cert_store_ = nullptr;
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        bool server_certificate_verification_ = true;
        bool server_hostname_verification_ = true;
        std::function<SSLVerifierResponse(SSL* ssl)> server_certificate_verifier_;
#endif

        Logger logger_;

    private:
        auto send_(Request& req, Response& res, Error& error) -> bool;
        auto send_(Request&& req) -> Result;

        auto create_client_socket(Error& error) const -> socket_t;
        static auto read_response_line(Stream& strm, const Request& req, Response& res) -> bool;
        auto write_request(Stream& strm, Request& req, bool close_connection, Error& error) const -> bool;
        auto redirect(Request& req, Response& res, Error& error) -> bool;
        auto handle_request(Stream& strm, Request& req, Response& res, bool close_connection, Error& error) -> bool;
        auto send_with_content_provider(Request& req,
                                        const char* body,
                                        size_t content_length,
                                        ContentProvider content_provider,
                                        ContentProviderWithoutLength content_provider_without_length,
                                        const std::string& content_type,
                                        Error& error) -> std::unique_ptr<Response>;
        auto send_with_content_provider(const std::string& method,
                                        const std::string& path,
                                        const Headers& headers,
                                        const char* body,
                                        size_t content_length,
                                        ContentProvider content_provider,
                                        ContentProviderWithoutLength content_provider_without_length,
                                        const std::string& content_type,
                                        const Progress& progress) -> Result;
        static auto get_multipart_content_provider(const std::string& boundary,
                                                   const MultipartFormDataItems& items,
                                                   const MultipartFormDataProviderItems& provider_items) -> ContentProviderWithoutLength;

        static auto adjust_host_string(const std::string& host) -> std::string;

        virtual auto process_socket(const Socket& socket,
                                    std::chrono::time_point<std::chrono::steady_clock> start_time,
                                    std::function<bool(Stream& strm)> callback) -> bool;
        virtual auto is_ssl() const -> bool;
    };

    class Client {
    public:
        // Universal interface
        explicit Client(const std::string& scheme_host_port);

        explicit Client(const std::string& scheme_host_port, const std::string& client_cert_path, const std::string& client_key_path);

        // HTTP only interface
        explicit Client(const std::string& host, int port);

        explicit Client(const std::string& host, int port, const std::string& client_cert_path, const std::string& client_key_path);

        Client(Client&&) = default;
        auto operator=(Client&&) -> Client& = default;

        ~Client();

        [[nodiscard]] auto is_valid() const -> bool;

        [[nodiscard]] auto Get(const std::string& path) const -> Result;
        [[nodiscard]] auto Get(const std::string& path, const Headers& headers) const -> Result;
        [[nodiscard]] auto Get(const std::string& path, Progress progress) const -> Result;
        [[nodiscard]] auto Get(const std::string& path, const Headers& headers, Progress progress) const -> Result;
        [[nodiscard]] auto Get(const std::string& path, const ContentReceiver& content_receiver) const -> Result;
        [[nodiscard]] auto Get(const std::string& path, const Headers& headers, const ContentReceiver& content_receiver) const -> Result;
        [[nodiscard]] auto Get(const std::string& path, const ContentReceiver& content_receiver, Progress progress) const -> Result;
        [[nodiscard]] auto Get(const std::string& path, const Headers& headers, const ContentReceiver& content_receiver, Progress progress) const -> Result;
        [[nodiscard]] auto Get(const std::string& path, ResponseHandler response_handler, const ContentReceiver& content_receiver) const -> Result;
        [[nodiscard]] auto Get(const std::string& path,
                               const Headers& headers,
                               ResponseHandler response_handler,
                               const ContentReceiver& content_receiver) const -> Result;
        [[nodiscard]] auto Get(const std::string& path,
                               const Headers& headers,
                               ResponseHandler response_handler,
                               const ContentReceiver& content_receiver,
                               Progress progress) const -> Result;
        [[nodiscard]] auto Get(const std::string& path,
                               ResponseHandler response_handler,
                               const ContentReceiver& content_receiver,
                               Progress progress) const -> Result;

        [[nodiscard]] auto Get(const std::string& path, const Params& params, const Headers& headers, Progress progress = nullptr) const -> Result;
        [[nodiscard]] auto Get(const std::string& path,
                               const Params& params,
                               const Headers& headers,
                               const ContentReceiver& content_receiver,
                               Progress progress = nullptr) const -> Result;
        [[nodiscard]] auto Get(const std::string& path,
                               const Params& params,
                               const Headers& headers,
                               ResponseHandler response_handler,
                               const ContentReceiver& content_receiver,
                               Progress progress = nullptr) const -> Result;

        [[nodiscard]] auto Head(const std::string& path) const -> Result;
        [[nodiscard]] auto Head(const std::string& path, const Headers& headers) const -> Result;

        [[nodiscard]] auto Post(const std::string& path) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const Headers& headers) const -> Result;
        auto Post(const std::string& path, const char* body, size_t content_length, const std::string& content_type) const -> Result;
        auto Post(const std::string& path,
                  const Headers& headers,
                  const char* body,
                  size_t content_length,
                  const std::string& content_type) const -> Result;
        auto Post(const std::string& path,
                  const Headers& headers,
                  const char* body,
                  size_t content_length,
                  const std::string& content_type,
                  const Progress& progress) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const std::string& body, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const std::string& body, const std::string& content_type, const Progress& progress) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Post(const std::string& path,
                                const Headers& headers,
                                const std::string& body,
                                const std::string& content_type,
                                const Progress& progress) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, size_t content_length, ContentProvider content_provider, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Post(const std::string& path,
                                const Headers& headers,
                                size_t content_length,
                                ContentProvider content_provider,
                                const std::string& content_type) const -> Result;
        [[nodiscard]] auto Post(const std::string& path,
                                const Headers& headers,
                                ContentProviderWithoutLength content_provider,
                                const std::string& content_type) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const Params& params) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const Headers& headers, const Params& params) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const Headers& headers, const Params& params, const Progress& progress) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const MultipartFormDataItems& items) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const Headers& headers, const MultipartFormDataItems& items) const -> Result;
        [[nodiscard]] auto Post(const std::string& path, const Headers& headers, const MultipartFormDataItems& items, const std::string& boundary) const -> Result;
        [[nodiscard]] auto Post(const std::string& path,
                                const Headers& headers,
                                const MultipartFormDataItems& items,
                                const MultipartFormDataProviderItems& provider_items) const -> Result;

        [[nodiscard]] auto Put(const std::string& path) const -> Result;
        auto Put(const std::string& path, const char* body, size_t content_length, const std::string& content_type) const -> Result;
        auto Put(const std::string& path,
                 const Headers& headers,
                 const char* body,
                 size_t content_length,
                 const std::string& content_type) const -> Result;
        auto Put(const std::string& path,
                 const Headers& headers,
                 const char* body,
                 size_t content_length,
                 const std::string& content_type,
                 const Progress& progress) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, const std::string& body, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, const std::string& body, const std::string& content_type, const Progress& progress) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Put(const std::string& path,
                               const Headers& headers,
                               const std::string& body,
                               const std::string& content_type,
                               const Progress& progress) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, size_t content_length, ContentProvider content_provider, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Put(const std::string& path,
                               const Headers& headers,
                               size_t content_length,
                               ContentProvider content_provider,
                               const std::string& content_type) const -> Result;
        [[nodiscard]] auto Put(const std::string& path,
                               const Headers& headers,
                               ContentProviderWithoutLength content_provider,
                               const std::string& content_type) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, const Params& params) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, const Headers& headers, const Params& params) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, const Headers& headers, const Params& params, const Progress& progress) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, const MultipartFormDataItems& items) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, const Headers& headers, const MultipartFormDataItems& items) const -> Result;
        [[nodiscard]] auto Put(const std::string& path, const Headers& headers, const MultipartFormDataItems& items, const std::string& boundary) const -> Result;
        [[nodiscard]] auto Put(const std::string& path,
                               const Headers& headers,
                               const MultipartFormDataItems& items,
                               const MultipartFormDataProviderItems& provider_items) const -> Result;

        [[nodiscard]] auto Patch(const std::string& path) const -> Result;
        auto Patch(const std::string& path, const char* body, size_t content_length, const std::string& content_type) const -> Result;
        auto Patch(const std::string& path,
                   const char* body,
                   size_t content_length,
                   const std::string& content_type,
                   const Progress& progress) const -> Result;
        auto Patch(const std::string& path,
                   const Headers& headers,
                   const char* body,
                   size_t content_length,
                   const std::string& content_type) const -> Result;
        auto Patch(const std::string& path,
                   const Headers& headers,
                   const char* body,
                   size_t content_length,
                   const std::string& content_type,
                   const Progress& progress) const -> Result;
        [[nodiscard]] auto Patch(const std::string& path, const std::string& body, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Patch(const std::string& path, const std::string& body, const std::string& content_type, const Progress& progress) const -> Result;
        [[nodiscard]] auto Patch(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Patch(const std::string& path,
                                 const Headers& headers,
                                 const std::string& body,
                                 const std::string& content_type,
                                 const Progress& progress) const -> Result;
        [[nodiscard]] auto Patch(const std::string& path, size_t content_length, ContentProvider content_provider, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Patch(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Patch(const std::string& path,
                                 const Headers& headers,
                                 size_t content_length,
                                 ContentProvider content_provider,
                                 const std::string& content_type) const -> Result;
        [[nodiscard]] auto Patch(const std::string& path,
                                 const Headers& headers,
                                 ContentProviderWithoutLength content_provider,
                                 const std::string& content_type) const -> Result;

        [[nodiscard]] auto Delete(const std::string& path) const -> Result;
        [[nodiscard]] auto Delete(const std::string& path, const Headers& headers) const -> Result;
        auto Delete(const std::string& path, const char* body, size_t content_length, const std::string& content_type) const -> Result;
        auto Delete(const std::string& path,
                    const char* body,
                    size_t content_length,
                    const std::string& content_type,
                    const Progress& progress) const -> Result;
        auto Delete(const std::string& path,
                    const Headers& headers,
                    const char* body,
                    size_t content_length,
                    const std::string& content_type) const -> Result;
        auto Delete(const std::string& path,
                    const Headers& headers,
                    const char* body,
                    size_t content_length,
                    const std::string& content_type,
                    const Progress& progress) const -> Result;
        [[nodiscard]] auto Delete(const std::string& path, const std::string& body, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Delete(const std::string& path, const std::string& body, const std::string& content_type, const Progress& progress) const -> Result;
        [[nodiscard]] auto Delete(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) const -> Result;
        [[nodiscard]] auto Delete(const std::string& path,
                                  const Headers& headers,
                                  const std::string& body,
                                  const std::string& content_type,
                                  const Progress& progress) const -> Result;

        [[nodiscard]] auto Options(const std::string& path) const -> Result;
        [[nodiscard]] auto Options(const std::string& path, const Headers& headers) const -> Result;

        auto send(Request& req, Response& res, Error& error) const -> bool;
        [[nodiscard]] auto send(const Request& req) const -> Result;

        auto stop() const -> void;

        [[nodiscard]] auto host() const -> std::string;
        [[nodiscard]] auto port() const -> int;

        [[nodiscard]] auto is_socket_open() const -> size_t;
        [[nodiscard]] auto socket() const -> socket_t;

        auto set_hostname_addr_map(std::map<std::string, std::string> addr_map) const -> void;

        auto set_default_headers(Headers headers) const -> void;

        auto set_header_writer(const std::function<ssize_t(Stream&, Headers&)>& writer) const -> void;

        auto set_address_family(int family) const -> void;
        auto set_tcp_nodelay(bool on) const -> void;
        auto set_socket_options(SocketOptions socket_options) const -> void;

        auto set_connection_timeout(time_t sec, time_t usec = 0) const -> void;
        template <class Rep, class Period>
        auto set_connection_timeout(const std::chrono::duration<Rep, Period>& duration) -> void;

        auto set_read_timeout(time_t sec, time_t usec = 0) const -> void;
        template <class Rep, class Period>
        auto set_read_timeout(const std::chrono::duration<Rep, Period>& duration) -> void;

        auto set_write_timeout(time_t sec, time_t usec = 0) const -> void;
        template <class Rep, class Period>
        auto set_write_timeout(const std::chrono::duration<Rep, Period>& duration) -> void;

        auto set_max_timeout(time_t msec) -> void;
        template <class Rep, class Period>
        auto set_max_timeout(const std::chrono::duration<Rep, Period>& duration) -> void;

        auto set_basic_auth(const std::string& username, const std::string& password) const -> void;
        auto set_bearer_token_auth(const std::string& token) const -> void;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto set_digest_auth(const std::string& username, const std::string& password) const -> void;
#endif

        auto set_keep_alive(bool on) const -> void;
        auto set_follow_location(bool on) const -> void;

        auto set_url_encode(bool on) const -> void;

        auto set_compress(bool on) const -> void;

        auto set_decompress(bool on) const -> void;

        auto set_interface(const std::string& intf) const -> void;

        auto set_proxy(const std::string& host, int port) const -> void;
        auto set_proxy_basic_auth(const std::string& username, const std::string& password) const -> void;
        auto set_proxy_bearer_token_auth(const std::string& token) const -> void;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto set_proxy_digest_auth(const std::string& username, const std::string& password) const -> void;
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto enable_server_certificate_verification(bool enabled) const -> void;
        auto enable_server_hostname_verification(bool enabled) const -> void;
        auto set_server_certificate_verifier(std::function<SSLVerifierResponse(SSL* ssl)> verifier) const -> void;
#endif

        auto set_logger(Logger logger) const -> void;

        // SSL
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto set_ca_cert_path(const std::string& ca_cert_file_path, const std::string& ca_cert_dir_path = std::string()) const -> void;

        auto set_ca_cert_store(X509_STORE* ca_cert_store) const -> void;
        auto load_ca_cert_store(const char* ca_cert, std::size_t size) const -> void;

        [[nodiscard]] auto get_openssl_verify_result() const -> long;

        [[nodiscard]] auto ssl_context() const -> SSL_CTX*;
#endif

    private:
        std::unique_ptr<ClientImpl> cli_;

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        bool is_ssl_ = false;
#endif
    };

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    class SSLServer final : public Server {
    public:
        SSLServer(const char* cert_path,
                  const char* private_key_path,
                  const char* client_ca_cert_file_path = nullptr,
                  const char* client_ca_cert_dir_path = nullptr,
                  const char* private_key_password = nullptr);

        SSLServer(X509* cert, EVP_PKEY* private_key, X509_STORE* client_ca_cert_store = nullptr);

        SSLServer(const std::function<bool(SSL_CTX& ssl_ctx)>& setup_ssl_ctx_callback);

        ~SSLServer() override;

        auto is_valid() const -> bool override;

        auto ssl_context() const -> SSL_CTX*;

        auto update_certs(X509* cert, EVP_PKEY* private_key, X509_STORE* client_ca_cert_store = nullptr) -> void;

    private:
        auto process_and_close_socket(socket_t sock, const std::string& ip, int port) -> bool override;

        SSL_CTX* ctx_;
        std::mutex ctx_mutex_;
    };

    class SSLClient final : public ClientImpl {
    public:
        explicit SSLClient(const std::string& host);

        explicit SSLClient(const std::string& host, int port);

        explicit SSLClient(const std::string& host,
                           int port,
                           const std::string& client_cert_path,
                           const std::string& client_key_path,
                           const std::string& private_key_password = std::string());

        explicit SSLClient(const std::string& host,
                           int port,
                           X509* client_cert,
                           EVP_PKEY* client_key,
                           const std::string& private_key_password = std::string());

        ~SSLClient() override;

        auto is_valid() const -> bool override;

        auto set_ca_cert_store(X509_STORE* ca_cert_store) const -> void;
        auto load_ca_cert_store(const char* ca_cert, std::size_t size) const -> void;

        auto get_openssl_verify_result() const -> long;

        auto ssl_context() const -> SSL_CTX*;

    private:
        auto create_and_connect_socket(Socket& socket, Error& error) -> bool override;
        auto shutdown_ssl(Socket& socket, bool shutdown_gracefully) -> void override;
        auto shutdown_ssl_impl(Socket& socket, bool shutdown_gracefully) -> void;

        auto process_socket(const Socket& socket,
                            std::chrono::time_point<std::chrono::steady_clock> start_time,
                            std::function<bool(Stream& strm)> callback) -> bool override;
        auto is_ssl() const -> bool override;

        auto connect_with_proxy(Socket& sock,
                                std::chrono::time_point<std::chrono::steady_clock> start_time,
                                Response& res,
                                bool& success,
                                Error& error) -> bool;
        auto initialize_ssl(Socket& socket, Error& error) -> bool;

        auto load_certs() -> bool;

        auto verify_host(X509* server_cert) const -> bool;
        auto verify_host_with_subject_alt_name(const X509* server_cert) const -> bool;
        auto verify_host_with_common_name(const X509* server_cert) const -> bool;
        auto check_host_name(const char* pattern, size_t pattern_len) const -> bool;

        SSL_CTX* ctx_;
        std::mutex ctx_mutex_;
        std::once_flag initialize_cert_;

        std::vector<std::string> host_components_;

        long verify_result_ = 0;

        friend class ClientImpl;
    };
#endif

    /*
     * Implementation of template methods.
     */

    namespace detail {
        template <typename T, typename U>
        auto duration_to_sec_and_usec(const T& duration, U callback) -> void {
            auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
            auto usec = std::chrono::duration_cast<std::chrono::microseconds>(duration - std::chrono::seconds(sec)).count();
            callback(static_cast<time_t>(sec), static_cast<time_t>(usec));
        }

        inline auto is_numeric(const std::string& str) -> bool { return !str.empty() && std::ranges::all_of(str, isdigit); }

        inline auto get_header_value_u64(const Headers& headers,
                                         const std::string& key,
                                         const uint64_t def,
                                         const size_t id,
                                         bool& is_invalid_value) -> uint64_t {
            is_invalid_value = false;
            const auto rng = headers.equal_range(key);
            auto it = rng.first;
            std::advance(it, static_cast<ssize_t>(id));
            if (it != rng.second) {
                if (is_numeric(it->second)) {
                    return std::strtoull(it->second.data(), nullptr, 10);
                }
                is_invalid_value = true;
            }
            return def;
        }

        inline auto get_header_value_u64(const Headers& headers, const std::string& key, const uint64_t def, const size_t id) -> uint64_t {
            bool dummy = false;
            return get_header_value_u64(headers, key, def, id, dummy);
        }
    } // namespace detail

    inline auto Request::get_header_value_u64(const std::string& key, const uint64_t def, const size_t id) const -> uint64_t {
        return detail::get_header_value_u64(headers, key, def, id);
    }

    inline auto Response::get_header_value_u64(const std::string& key, const uint64_t def, const size_t id) const -> uint64_t {
        return detail::get_header_value_u64(headers, key, def, id);
    }

    inline auto default_socket_options(socket_t sock) -> void {
        int opt = 1;
#ifdef _WIN32
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
#ifdef SO_REUSEPORT
  setsockopt(sock, SOL_SOCKET, SO_REUSEPORT,
             reinterpret_cast<const void *>(&opt), sizeof(opt));
#else
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
             reinterpret_cast<const void *>(&opt), sizeof(opt));
#endif
#endif
    }

    inline auto status_message(const int status) -> const char* {
        switch (status) {
            case Continue_100:
                return "Continue";
            case SwitchingProtocol_101:
                return "Switching Protocol";
            case Processing_102:
                return "Processing";
            case EarlyHints_103:
                return "Early Hints";
            case OK_200:
                return "OK";
            case Created_201:
                return "Created";
            case Accepted_202:
                return "Accepted";
            case NonAuthoritativeInformation_203:
                return "Non-Authoritative Information";
            case NoContent_204:
                return "No Content";
            case ResetContent_205:
                return "Reset Content";
            case PartialContent_206:
                return "Partial Content";
            case MultiStatus_207:
                return "Multi-Status";
            case AlreadyReported_208:
                return "Already Reported";
            case IMUsed_226:
                return "IM Used";
            case MultipleChoices_300:
                return "Multiple Choices";
            case MovedPermanently_301:
                return "Moved Permanently";
            case Found_302:
                return "Found";
            case SeeOther_303:
                return "See Other";
            case NotModified_304:
                return "Not Modified";
            case UseProxy_305:
                return "Use Proxy";
            case unused_306:
                return "unused";
            case TemporaryRedirect_307:
                return "Temporary Redirect";
            case PermanentRedirect_308:
                return "Permanent Redirect";
            case BadRequest_400:
                return "Bad Request";
            case Unauthorized_401:
                return "Unauthorized";
            case PaymentRequired_402:
                return "Payment Required";
            case Forbidden_403:
                return "Forbidden";
            case NotFound_404:
                return "Not Found";
            case MethodNotAllowed_405:
                return "Method Not Allowed";
            case NotAcceptable_406:
                return "Not Acceptable";
            case ProxyAuthenticationRequired_407:
                return "Proxy Authentication Required";
            case RequestTimeout_408:
                return "Request Timeout";
            case Conflict_409:
                return "Conflict";
            case Gone_410:
                return "Gone";
            case LengthRequired_411:
                return "Length Required";
            case PreconditionFailed_412:
                return "Precondition Failed";
            case PayloadTooLarge_413:
                return "Payload Too Large";
            case UriTooLong_414:
                return "URI Too Long";
            case UnsupportedMediaType_415:
                return "Unsupported Media Type";
            case RangeNotSatisfiable_416:
                return "Range Not Satisfiable";
            case ExpectationFailed_417:
                return "Expectation Failed";
            case ImATeapot_418:
                return "I'm a teapot";
            case MisdirectedRequest_421:
                return "Misdirected Request";
            case UnprocessableContent_422:
                return "Unprocessable Content";
            case Locked_423:
                return "Locked";
            case FailedDependency_424:
                return "Failed Dependency";
            case TooEarly_425:
                return "Too Early";
            case UpgradeRequired_426:
                return "Upgrade Required";
            case PreconditionRequired_428:
                return "Precondition Required";
            case TooManyRequests_429:
                return "Too Many Requests";
            case RequestHeaderFieldsTooLarge_431:
                return "Request Header Fields Too Large";
            case UnavailableForLegalReasons_451:
                return "Unavailable For Legal Reasons";
            case NotImplemented_501:
                return "Not Implemented";
            case BadGateway_502:
                return "Bad Gateway";
            case ServiceUnavailable_503:
                return "Service Unavailable";
            case GatewayTimeout_504:
                return "Gateway Timeout";
            case HttpVersionNotSupported_505:
                return "HTTP Version Not Supported";
            case VariantAlsoNegotiates_506:
                return "Variant Also Negotiates";
            case InsufficientStorage_507:
                return "Insufficient Storage";
            case LoopDetected_508:
                return "Loop Detected";
            case NotExtended_510:
                return "Not Extended";
            case NetworkAuthenticationRequired_511:
                return "Network Authentication Required";

            default: case InternalServerError_500:
                return "Internal Server Error";
        }
    }

    inline auto get_bearer_token_auth(const Request& req) -> std::string {
        if (req.has_header("Authorization")) {
            static std::string BearerHeaderPrefix = "Bearer ";
            return req.get_header_value("Authorization").substr(BearerHeaderPrefix.length());
        }
        return "";
    }

    template <class Rep, class Period>
    auto Server::set_read_timeout(const std::chrono::duration<Rep, Period>& duration) -> Server& {
        detail::duration_to_sec_and_usec(duration,
                                         [&](const time_t sec, const time_t usec) { set_read_timeout(sec, usec); });
        return *this;
    }

    template <class Rep, class Period>
    auto Server::set_write_timeout(const std::chrono::duration<Rep, Period>& duration) -> Server& {
        detail::duration_to_sec_and_usec(duration,
                                         [&](const time_t sec, const time_t usec) { set_write_timeout(sec, usec); });
        return *this;
    }

    template <class Rep, class Period>
    auto Server::set_idle_interval(const std::chrono::duration<Rep, Period>& duration) -> Server& {
        detail::duration_to_sec_and_usec(duration,
                                         [&](const time_t sec, const time_t usec) { set_idle_interval(sec, usec); });
        return *this;
    }

    inline auto to_string(const Error error) -> std::string {
        switch (error) {
            case Error::Success:
                return "Success (no error)";
            case Error::Connection:
                return "Could not establish connection";
            case Error::BindIPAddress:
                return "Failed to bind IP address";
            case Error::Read:
                return "Failed to read connection";
            case Error::Write:
                return "Failed to write connection";
            case Error::ExceedRedirectCount:
                return "Maximum redirect count exceeded";
            case Error::Canceled:
                return "Connection handling canceled";
            case Error::SSLConnection:
                return "SSL connection failed";
            case Error::SSLLoadingCerts:
                return "SSL certificate loading failed";
            case Error::SSLServerVerification:
                return "SSL server verification failed";
            case Error::SSLServerHostnameVerification:
                return "SSL server hostname verification failed";
            case Error::UnsupportedMultipartBoundaryChars:
                return "Unsupported HTTP multipart boundary characters";
            case Error::Compression:
                return "Compression failed";
            case Error::ConnectionTimeout:
                return "Connection timed out";
            case Error::ProxyConnection:
                return "Proxy connection failed";
            case Error::Unknown:
                return "Unknown";
            default:
                break;
        }

        return "Invalid";
    }

    inline auto operator<<(std::ostream& os, const Error& obj) -> std::ostream& {
        os << to_string(obj);
        os << " (" << static_cast<std::underlying_type_t<Error>>(obj) << ')';
        return os;
    }

    inline auto Result::get_request_header_value_u64(const std::string& key, const uint64_t def, const size_t id) const -> uint64_t {
        return detail::get_header_value_u64(request_headers_, key, def, id);
    }

    template <class Rep, class Period>
    auto ClientImpl::set_connection_timeout(const std::chrono::duration<Rep, Period>& duration) -> void {
        detail::duration_to_sec_and_usec(duration,
                                         [&](const time_t sec, const time_t usec) { set_connection_timeout(sec, usec); });
    }

    template <class Rep, class Period>
    auto ClientImpl::set_read_timeout(const std::chrono::duration<Rep, Period>& duration) -> void {
        detail::duration_to_sec_and_usec(duration,
                                         [&](const time_t sec, const time_t usec) { set_read_timeout(sec, usec); });
    }

    template <class Rep, class Period>
    auto ClientImpl::set_write_timeout(const std::chrono::duration<Rep, Period>& duration) -> void {
        detail::duration_to_sec_and_usec(duration,
                                         [&](const time_t sec, const time_t usec) { set_write_timeout(sec, usec); });
    }

    template <class Rep, class Period>
    auto ClientImpl::set_max_timeout(const std::chrono::duration<Rep, Period>& duration) -> void {
        auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        set_max_timeout(msec);
    }

    template <class Rep, class Period>
    auto Client::set_connection_timeout(const std::chrono::duration<Rep, Period>& duration) -> void { cli_->set_connection_timeout(duration); }

    template <class Rep, class Period>
    auto Client::set_read_timeout(const std::chrono::duration<Rep, Period>& duration) -> void { cli_->set_read_timeout(duration); }

    template <class Rep, class Period>
    auto Client::set_write_timeout(const std::chrono::duration<Rep, Period>& duration) -> void { cli_->set_write_timeout(duration); }

    template <class Rep, class Period>
    auto Client::set_max_timeout(const std::chrono::duration<Rep, Period>& duration) -> void { cli_->set_max_timeout(duration); }

    /*
     * Forward declarations and types that will be part of the .h file if split into
     * .h + .cc.
     */

    auto hosted_at(const std::string& hostname) -> std::string;

    auto hosted_at(const std::string& hostname, std::vector<std::string>& addrs) -> void;

    auto append_query_params(const std::string& path, const Params& params) -> std::string;

    auto make_range_header(const Ranges& ranges) -> std::pair<std::string, std::string>;

    auto make_basic_authentication_header(const std::string& username,
                                          const std::string& password,
                                          bool is_proxy = false) -> std::pair<std::string, std::string>;

    namespace detail {
#if defined(_WIN32)
        inline auto u8string_to_wstring(const char* s) -> std::wstring {
            std::wstring ws;
            const auto len = static_cast<int>(strlen(s));
            auto wlen = MultiByteToWideChar(CP_UTF8, 0, s, len, nullptr, 0);
            if (wlen > 0) {
                ws.resize(wlen);
                wlen = MultiByteToWideChar(CP_UTF8, 0, s, len, const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(ws.data())), wlen);
                if (wlen != static_cast<int>(ws.size())) {
                    ws.clear();
                }
            }
            return ws;
        }
#endif

        struct FileStat {
            FileStat(const std::string& path);
            [[nodiscard]] auto is_file() const -> bool;
            [[nodiscard]] auto is_dir() const -> bool;

        private:
#if defined(_WIN32)
            struct _stat st_;
#else
  struct stat st_;
#endif
            int ret_ = -1;
        };

        auto encode_query_param(const std::string& value) -> std::string;

        auto decode_url(const std::string& s, bool convert_plus_to_space) -> std::string;

        auto read_file(const std::string& path, std::string& out) -> void;

        auto trim_copy(const std::string& s) -> std::string;

        auto divide(const char* data,
                    std::size_t size,
                    char d,
                    const std::function<void(const char*, std::size_t, const char*, std::size_t)>& fn) -> void;

        auto divide(const std::string& str, char d, const std::function<void(const char*, std::size_t, const char*, std::size_t)>& fn) -> void;

        auto split(const char* b, const char* e, char d, const std::function<void(const char*, const char*)>& fn) -> void;

        auto split(const char* b, const char* e, char d, size_t m, const std::function<void(const char*, const char*)>& fn) -> void;

        auto process_client_socket(socket_t sock,
                                   time_t read_timeout_sec,
                                   time_t read_timeout_usec,
                                   time_t write_timeout_sec,
                                   time_t write_timeout_usec,
                                   time_t max_timeout_msec,
                                   std::chrono::time_point<std::chrono::steady_clock> start_time,
                                   const std::function<bool(Stream&)>& callback) -> bool;

        auto create_client_socket(const std::string& host,
                                  const std::string& ip,
                                  int port,
                                  int address_family,
                                  bool tcp_nodelay,
                                  bool ipv6_v6only,
                                  const SocketOptions& socket_options,
                                  time_t connection_timeout_sec,
                                  time_t connection_timeout_usec,
                                  time_t read_timeout_sec,
                                  time_t read_timeout_usec,
                                  time_t write_timeout_sec,
                                  time_t write_timeout_usec,
                                  const std::string& intf,
                                  Error& error) -> socket_t;

        auto get_header_value(const Headers& headers, const std::string& key, const char* def, size_t id) -> const char*;

        auto params_to_query_str(const Params& params) -> std::string;

        auto parse_query_text(const char* data, std::size_t size, Params& params) -> void;

        auto parse_query_text(const std::string& s, Params& params) -> void;

        auto parse_multipart_boundary(const std::string& content_type, std::string& boundary) -> bool;

        auto parse_range_header(const std::string& s, Ranges& ranges) -> bool;

        auto close_socket(socket_t sock) -> int;

        auto send_socket(socket_t sock, const void* ptr, size_t size, int flags) -> ssize_t;

        auto read_socket(socket_t sock, void* ptr, size_t size, int flags) -> ssize_t;

        enum class EncodingType { None = 0, Gzip, Brotli };

        auto encoding_type(const Request& req, const Response& res) -> EncodingType;

        class BufferStream final : public Stream {
        public:
            BufferStream() = default;
            ~BufferStream() override = default;

            [[nodiscard]] auto is_readable() const -> bool override;
            [[nodiscard]] auto is_writable() const -> bool override;
            auto read(char* ptr, size_t size) -> ssize_t override;
            auto write(const char* ptr, size_t size) -> ssize_t override;
            auto get_remote_ip_and_port(std::string& ip, int& port) const -> void override;
            auto get_local_ip_and_port(std::string& ip, int& port) const -> void override;
            [[nodiscard]] auto socket() const -> socket_t override;
            [[nodiscard]] auto duration() const -> time_t override;

            [[nodiscard]] auto get_buffer() const -> const std::string&;

        private:
            std::string buffer;
            size_t position = 0;
        };

        class compressor {
        public:
            virtual ~compressor() = default;

            using Callback = std::function<bool(const char* data, size_t data_len)>;
            virtual auto compress(const char* data, size_t data_length, bool last, Callback callback) -> bool = 0;
        };

        class decompressor {
        public:
            virtual ~decompressor() = default;

            [[nodiscard]] virtual auto is_valid() const -> bool = 0;

            using Callback = std::function<bool(const char* data, size_t data_len)>;
            virtual auto decompress(const char* data, size_t data_length, Callback callback) -> bool = 0;
        };

        class nocompressor final : public compressor {
        public:
            ~nocompressor() override = default;

            auto compress(const char* data, size_t data_length, bool /*last*/, Callback callback) -> bool override;
        };

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
        class gzip_compressor final : public compressor {
        public:
            gzip_compressor();
            ~gzip_compressor() override;

            auto compress(const char* data, size_t data_length, bool last, Callback callback) -> bool override;

        private:
            bool is_valid_ = false;
            z_stream strm_;
        };

        class gzip_decompressor final : public decompressor {
        public:
            gzip_decompressor();
            ~gzip_decompressor() override;

            [[nodiscard]] auto is_valid() const -> bool override;

            auto decompress(const char* data, size_t data_length, Callback callback) -> bool override;

        private:
            bool is_valid_ = false;
            z_stream strm_;
        };
#endif

        // NOTE: until the read size reaches `fixed_buffer_size`, use `fixed_buffer`
        // to store data. The call can set memory on stack for performance.
        class stream_line_reader {
        public:
            stream_line_reader(Stream& strm, char* fixed_buffer, size_t fixed_buffer_size);
            [[nodiscard]] auto ptr() const -> const char*;
            [[nodiscard]] auto size() const -> size_t;
            [[nodiscard]] auto end_with_crlf() const -> bool;
            auto getline() -> bool;

        private:
            auto append(char c) -> void;

            Stream& strm_;
            char* fixed_buffer_;
            const size_t fixed_buffer_size_;
            size_t fixed_buffer_used_size_ = 0;
            std::string glowable_buffer_;
        };

        class mmap {
        public:
            mmap(const char* path);
            ~mmap();

            auto open(const char* path) -> bool;
            auto close() -> void;

            [[nodiscard]] auto is_open() const -> bool;
            [[nodiscard]] auto size() const -> size_t;
            [[nodiscard]] auto data() const -> const char*;

        private:
#if defined(_WIN32)
            HANDLE hFile_ = nullptr;
            HANDLE hMapping_ = nullptr;
#else
  int fd_ = -1;
#endif
            size_t size_ = 0;
            void* addr_ = nullptr;
            bool is_open_empty_file = false;
        };

        // NOTE: https://www.rfc-editor.org/rfc/rfc9110#section-5
        namespace fields {
            inline auto is_token_char(const char c) -> bool {
                return std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%' || c == '&' || c == '\'' || c == '*' || c == '+' || c == '-' ||
                    c == '.' || c == '^' || c ==
                    '_' || c == '`' || c == '|' || c == '~';
            }

            inline auto is_token(const std::string& s) -> bool {
                if (s.empty()) {
                    return false;
                }
                for (const auto c : s) {
                    if (!is_token_char(c)) {
                        return false;
                    }
                }
                return true;
            }

            inline auto is_field_name(const std::string& s) -> bool { return is_token(s); }

            inline auto is_vchar(const char c) -> bool { return c >= 33 && c <= 126; }

            inline auto is_obs_text(const char c) -> bool { return 128 <= static_cast<unsigned char>(c); }

            inline auto is_field_vchar(const char c) -> bool { return is_vchar(c) || is_obs_text(c); }

            inline auto is_field_content(const std::string& s) -> bool {
                if (s.empty()) {
                    return true;
                }

                if (s.size() == 1) {
                    return is_field_vchar(s[0]);
                }
                if (s.size() == 2) {
                    return is_field_vchar(s[0]) && is_field_vchar(s[1]);
                }
                size_t i = 0;

                if (!is_field_vchar(s[i])) {
                    return false;
                }
                i++;

                while (i < s.size() - 1) {
                    const auto c = s[i++];
                    if (c == ' ' || c == '\t' || is_field_vchar(c)) {} else {
                        return false;
                    }
                }

                return is_field_vchar(s[i]);
            }

            inline auto is_field_value(const std::string& s) -> bool { return is_field_content(s); }
        } // namespace fields
    } // namespace detail

    // ----------------------------------------------------------------------------

    /*
     * Implementation that will be part of the .cc file if split into .h + .cc.
     */

    namespace detail {
        inline auto is_hex(const char c, int& v) -> bool {
            if (0x20 <= c && isdigit(c)) {
                v = c - '0';
                return true;
            }
            if ('A' <= c && c <= 'F') {
                v = c - 'A' + 10;
                return true;
            }
            if ('a' <= c && c <= 'f') {
                v = c - 'a' + 10;
                return true;
            }
            return false;
        }

        inline auto from_hex_to_i(const std::string& s, size_t i, size_t cnt, int& val) -> bool {
            if (i >= s.size()) {
                return false;
            }

            val = 0;
            for (; cnt; i++, cnt--) {
                if (!s[i]) {
                    return false;
                }
                auto v = 0;
                if (is_hex(s[i], v)) {
                    val = val * 16 + v;
                } else {
                    return false;
                }
            }
            return true;
        }

        inline auto from_i_to_hex(size_t n) -> std::string {
            static const auto charset = "0123456789abcdef";
            std::string ret;
            do {
                ret = charset[n & 15] + ret;
                n >>= 4;
            } while (n > 0);
            return ret;
        }

        inline auto to_utf8(const int code, char* buff) -> size_t {
            if (code < 0x0080) {
                buff[0] = static_cast<char>(code & 0x7F);
                return 1;
            }
            if (code < 0x0800) {
                buff[0] = static_cast<char>(0xC0 | code >> 6 & 0x1F);
                buff[1] = static_cast<char>(0x80 | code & 0x3F);
                return 2;
            }
            if (code < 0xD800) {
                buff[0] = static_cast<char>(0xE0 | code >> 12 & 0xF);
                buff[1] = static_cast<char>(0x80 | code >> 6 & 0x3F);
                buff[2] = static_cast<char>(0x80 | code & 0x3F);
                return 3;
            }
            if (code < 0xE000) {
                // D800 - DFFF is invalid...
                return 0;
            }
            if (code < 0x10000) {
                buff[0] = static_cast<char>(0xE0 | code >> 12 & 0xF);
                buff[1] = static_cast<char>(0x80 | code >> 6 & 0x3F);
                buff[2] = static_cast<char>(0x80 | code & 0x3F);
                return 3;
            }
            if (code < 0x110000) {
                buff[0] = static_cast<char>(0xF0 | code >> 18 & 0x7);
                buff[1] = static_cast<char>(0x80 | code >> 12 & 0x3F);
                buff[2] = static_cast<char>(0x80 | code >> 6 & 0x3F);
                buff[3] = static_cast<char>(0x80 | code & 0x3F);
                return 4;
            }

            // NOTREACHED
            return 0;
        }

        // NOTE: This code came up with the following stackoverflow post:
        // https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
        inline auto base64_encode(const std::string& in) -> std::string {
            static const auto lookup = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

            std::string out;
            out.reserve(in.size());

            auto val = 0;
            auto valb = -6;

            for (const auto c : in) {
                val = (val << 8) + static_cast<uint8_t>(c);
                valb += 8;
                while (valb >= 0) {
                    out.push_back(lookup[val >> valb & 0x3F]);
                    valb -= 6;
                }
            }

            if (valb > -6) {
                out.push_back(lookup[(val << 8) >> (valb + 8) & 0x3F]);
            }

            while (out.size() % 4) {
                out.push_back('=');
            }

            return out;
        }

        inline auto is_valid_path(const std::string& path) -> bool {
            size_t level = 0;
            size_t i = 0;

            // Skip slash
            while (i < path.size() && path[i] == '/') {
                i++;
            }

            while (i < path.size()) {
                // Read component
                const auto beg = i;
                while (i < path.size() && path[i] != '/') {
                    if (path[i] == '\0') {
                        return false;
                    }
                    if (path[i] == '\\') {
                        return false;
                    }
                    i++;
                }

                const auto len = i - beg;
                assert(len > 0);

                if (!path.compare(beg, len, ".")) {} else if (!path.compare(beg, len, "..")) {
                    if (level == 0) {
                        return false;
                    }
                    level--;
                } else {
                    level++;
                }

                // Skip slash
                while (i < path.size() && path[i] == '/') {
                    i++;
                }
            }

            return true;
        }

        inline FileStat::FileStat(const std::string& path) {
#if defined(_WIN32)
            const auto wpath = u8string_to_wstring(path.c_str());
            ret_ = _wstat(wpath.c_str(), &st_);
#else
  ret_ = stat(path.c_str(), &st_);
#endif
        }

        inline auto FileStat::is_file() const -> bool { return ret_ >= 0 && S_ISREG(st_.st_mode); }

        inline auto FileStat::is_dir() const -> bool { return ret_ >= 0 && S_ISDIR(st_.st_mode); }

        inline auto encode_query_param(const std::string& value) -> std::string {
            std::ostringstream escaped;
            escaped.fill('0');
            escaped << std::hex;

            for (const auto c : value) {
                if (std::isalnum(static_cast<uint8_t>(c)) || c == '-' || c == '_' || c == '.' || c == '!' || c == '~' || c == '*' || c == '\'' || c ==
                    '(' || c == ')') {
                    escaped << c;
                } else {
                    escaped << std::uppercase;
                    escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
                    escaped << std::nouppercase;
                }
            }

            return escaped.str();
        }

        inline auto encode_url(const std::string& s) -> std::string {
            std::string result;
            result.reserve(s.size());

            for (size_t i = 0; s[i]; i++) {
                switch (s[i]) {
                    case ' ':
                        result += "%20";
                        break;
                    case '+':
                        result += "%2B";
                        break;
                    case '\r':
                        result += "%0D";
                        break;
                    case '\n':
                        result += "%0A";
                        break;
                    case '\'':
                        result += "%27";
                        break;
                    case ',':
                        result += "%2C";
                        break;
                    // case ':': result += "%3A"; break; // ok? probably...
                    case ';':
                        result += "%3B";
                        break;
                    default:
                        const auto c = static_cast<uint8_t>(s[i]);
                        if (c >= 0x80) {
                            result += '%';
                            char hex[4];
                            const auto len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
                            assert(len == 2);
                            result.append(hex, static_cast<size_t>(len));
                        } else {
                            result += s[i];
                        }
                        break;
                }
            }

            return result;
        }

        inline auto decode_url(const std::string& s, const bool convert_plus_to_space) -> std::string {
            std::string result;

            for (size_t i = 0; i < s.size(); i++) {
                if (s[i] == '%' && i + 1 < s.size()) {
                    if (s[i + 1] == 'u') {
                        auto val = 0;
                        if (from_hex_to_i(s, i + 2, 4, val)) {
                            // 4 digits Unicode codes
                            char buff[4];
                            const size_t len = to_utf8(val, buff);
                            if (len > 0) {
                                result.append(buff, len);
                            }
                            i += 5; // 'u0000'
                        } else {
                            result += s[i];
                        }
                    } else {
                        auto val = 0;
                        if (from_hex_to_i(s, i + 1, 2, val)) {
                            // 2 digits hex codes
                            result += static_cast<char>(val);
                            i += 2; // '00'
                        } else {
                            result += s[i];
                        }
                    }
                } else if (convert_plus_to_space && s[i] == '+') {
                    result += ' ';
                } else {
                    result += s[i];
                }
            }

            return result;
        }

        inline auto read_file(const std::string& path, std::string& out) -> void {
            std::ifstream fs(path, std::ios_base::binary);
            fs.seekg(0, std::ios_base::end);
            const auto size = fs.tellg();
            fs.seekg(0);
            out.resize(size);
            fs.read(out.data(), size);
        }

        inline auto file_extension(const std::string& path) -> std::string {
            std::smatch match_results;
            static const std::regex re(R"(\.([a-zA-Z0-9]+)$)");

            if (std::regex_search(path, match_results, re)) {
                return match_results[1].str();
            }
            return {};
        }

        inline auto is_space_or_tab(const char c) -> bool { return c == ' ' || c == '\t'; }

        inline auto trim(const char* b, const char* e, size_t left, size_t right) -> std::pair<size_t, size_t> {
            while (b + left < e && is_space_or_tab(b[left])) {
                left++;
            }
            while (right > 0 && is_space_or_tab(b[right - 1])) {
                right--;
            }
            return std::make_pair(left, right);
        }

        inline auto trim_copy(const std::string& s) -> std::string {
            const auto r = trim(s.data(), s.data() + s.size(), 0, s.size());
            return s.substr(r.first, r.second - r.first);
        }

        inline auto trim_double_quotes_copy(const std::string& s) -> std::string {
            if (s.length() >= 2 && s.front() == '"' && s.back() == '"') {
                return s.substr(1, s.size() - 2);
            }
            return s;
        }

        inline auto divide(const char* data,
                           const std::size_t size,
                           const char d,
                           const std::function<void(const char*, std::size_t, const char*, std::size_t)>& fn) -> void {
            const auto it = std::find(data, data + size, d);
            const auto found = static_cast<std::size_t>(it != data + size);
            const auto lhs_data = data;
            const auto lhs_size = static_cast<std::size_t>(it - data);
            const auto rhs_data = it + found;
            const auto rhs_size = size - lhs_size - found;

            fn(lhs_data, lhs_size, rhs_data, rhs_size);
        }

        inline auto divide(const std::string& str,
                           const char d,
                           const std::function<void(const char*, std::size_t, const char*, std::size_t)>& fn) -> void {
            divide(str.data(), str.size(), d, fn);
        }

        inline auto split(const char* b, const char* e, const char d, const std::function<void(const char*, const char*)>& fn) -> void {
            return split(b, e, d, (std::numeric_limits<size_t>::max)(), fn);
        }

        inline auto split(const char* b,
                          const char* e,
                          const char d,
                          const size_t m,
                          const std::function<void(const char*, const char*)>& fn) -> void {
            size_t i = 0;
            size_t beg = 0;
            size_t count = 1;

            while (e ? b + i < e : b[i] != '\0') {
                if (b[i] == d && count < m) {
                    const auto r = trim(b, e, beg, i);
                    if (r.first < r.second) {
                        fn(&b[r.first], &b[r.second]);
                    }
                    beg = i + 1;
                    count++;
                }
                i++;
            }

            if (i) {
                const auto r = trim(b, e, beg, i);
                if (r.first < r.second) {
                    fn(&b[r.first], &b[r.second]);
                }
            }
        }

        inline stream_line_reader::stream_line_reader(Stream& strm, char* fixed_buffer, const size_t fixed_buffer_size) : strm_(strm),
            fixed_buffer_(fixed_buffer),
            fixed_buffer_size_(fixed_buffer_size) {}

        inline auto stream_line_reader::ptr() const -> const char* {
            if (glowable_buffer_.empty()) {
                return fixed_buffer_;
            }
            return glowable_buffer_.data();
        }

        inline auto stream_line_reader::size() const -> size_t {
            if (glowable_buffer_.empty()) {
                return fixed_buffer_used_size_;
            }
            return glowable_buffer_.size();
        }

        inline auto stream_line_reader::end_with_crlf() const -> bool {
            const auto end = ptr() + size();
            return size() >= 2 && end[-2] == '\r' && end[-1] == '\n';
        }

        inline auto stream_line_reader::getline() -> bool {
            fixed_buffer_used_size_ = 0;
            glowable_buffer_.clear();

#ifndef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
            char prev_byte = 0;
#endif

            for (size_t i = 0;; i++) {
                char byte;
                const auto n = strm_.read(&byte, 1);

                if (n < 0) {
                    return false;
                }
                if (n == 0) {
                    if (i == 0) {
                        return false;
                    }
                    break;
                }

                append(byte);

#ifdef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
    if (byte == '\n') { break; }
#else
                if (prev_byte == '\r' && byte == '\n') {
                    break;
                }
                prev_byte = byte;
#endif
            }

            return true;
        }

        inline auto stream_line_reader::append(const char c) -> void {
            if (fixed_buffer_used_size_ < fixed_buffer_size_ - 1) {
                fixed_buffer_[fixed_buffer_used_size_++] = c;
                fixed_buffer_[fixed_buffer_used_size_] = '\0';
            } else {
                if (glowable_buffer_.empty()) {
                    assert(fixed_buffer_[fixed_buffer_used_size_] == '\0');
                    glowable_buffer_.assign(fixed_buffer_, fixed_buffer_used_size_);
                }
                glowable_buffer_ += c;
            }
        }

        inline mmap::mmap(const char* path) { open(path); }

        inline mmap::~mmap() { close(); }

        inline auto mmap::open(const char* path) -> bool {
            close();

#if defined(_WIN32)
            auto wpath = u8string_to_wstring(path);
            if (wpath.empty()) {
                return false;
            }

#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
            hFile_ = CreateFile2(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
#else
  hFile_ = ::CreateFileW(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif

            if (hFile_ == INVALID_HANDLE_VALUE) {
                return false;
            }

            LARGE_INTEGER size{};
            if (!GetFileSizeEx(hFile_, &size)) {
                return false;
            }
            // If the following line doesn't compile due to QuadPart, update Windows SDK.
            // See:
            // https://github.com/yhirose/cpp-httplib/issues/1903#issuecomment-2316520721
            if (static_cast<ULONGLONG>(size.QuadPart) > (std::numeric_limits<decltype(size_)>::max)()) {
                // `size_t` might be 32-bits, on 32-bits Windows.
                return false;
            }
            size_ = static_cast<size_t>(size.QuadPart);

#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
            hMapping_ = CreateFileMappingFromApp(hFile_, nullptr, PAGE_READONLY, size_, nullptr);
#else
  hMapping_ = ::CreateFileMappingW(hFile_, NULL, PAGE_READONLY, 0, 0, NULL);
#endif

            // Special treatment for an empty file...
            if (hMapping_ == nullptr && size_ == 0) {
                close();
                is_open_empty_file = true;
                return true;
            }

            if (hMapping_ == nullptr) {
                close();
                return false;
            }

#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
            addr_ = MapViewOfFileFromApp(hMapping_, FILE_MAP_READ, 0, 0);
#else
  addr_ = ::MapViewOfFile(hMapping_, FILE_MAP_READ, 0, 0, 0);
#endif

            if (addr_ == nullptr) {
                close();
                return false;
            }
#else
  fd_ = ::open(path, O_RDONLY);
  if (fd_ == -1) { return false; }

  struct stat sb;
  if (fstat(fd_, &sb) == -1) {
    close();
    return false;
  }
  size_ = static_cast<size_t>(sb.st_size);

  addr_ = ::mmap(NULL, size_, PROT_READ, MAP_PRIVATE, fd_, 0);

  // Special treatment for an empty file...
  if (addr_ == MAP_FAILED && size_ == 0) {
    close();
    is_open_empty_file = true;
    return false;
  }
#endif

            return true;
        }

        inline auto mmap::is_open() const -> bool { return is_open_empty_file ? true : addr_ != nullptr; }

        inline auto mmap::size() const -> size_t { return size_; }

        inline auto mmap::data() const -> const char* { return is_open_empty_file ? "" : static_cast<const char*>(addr_); }

        inline auto mmap::close() -> void {
            if (addr_) {
                UnmapViewOfFile(addr_);
                addr_ = nullptr;
            }

            if (hMapping_) {
                CloseHandle(hMapping_);
                hMapping_ = nullptr;
            }

            if (hFile_ != INVALID_HANDLE_VALUE) {
                CloseHandle(hFile_);
                hFile_ = INVALID_HANDLE_VALUE;
            }

            is_open_empty_file = false;
            size_ = 0;
        }

        inline auto close_socket(const socket_t sock) -> int { return closesocket(sock); }

        template <typename T>
        auto handle_EINTR(T fn) -> ssize_t {
            ssize_t res = 0;
            while (true) {
                res = fn();
                if (res < 0 && errno == EINTR) {
                    std::this_thread::sleep_for(1ms);
                    continue;
                }
                break;
            }
            return res;
        }

        inline auto read_socket(const socket_t sock, void* ptr, const size_t size, const int flags) -> ssize_t {
            return handle_EINTR([&] {
                return recv(sock,
                            static_cast<char*>(ptr),
                            static_cast<int>(size),
                            flags);
            });
        }

        inline auto send_socket(const socket_t sock, const void* ptr, const size_t size, const int flags) -> ssize_t {
            return handle_EINTR([&] {
                return send(sock,
                            static_cast<const char*>(ptr),
                            static_cast<int>(size),
                            flags);
            });
        }

        template <bool Read>
        auto select_impl(const socket_t sock, const time_t sec, const time_t usec) -> ssize_t {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(sock, &fds);
            fd_set* rfds = Read ? &fds : nullptr;
            fd_set* wfds = Read ? nullptr : &fds;

            timeval tv;
            tv.tv_sec = static_cast<long>(sec);
            tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);

            return handle_EINTR([&] { return select(static_cast<int>(sock + 1), rfds, wfds, nullptr, &tv); });
        }

        inline auto select_read(const socket_t sock, const time_t sec, const time_t usec) -> ssize_t { return select_impl<true>(sock, sec, usec); }

        inline auto select_write(const socket_t sock, const time_t sec, const time_t usec) -> ssize_t { return select_impl<false>(sock, sec, usec); }

        inline auto wait_until_socket_is_ready(const socket_t sock, const time_t sec, const time_t usec) -> Error {
            fd_set fdsr;
            FD_ZERO(&fdsr);
            FD_SET(sock, &fdsr);

            auto fdsw = fdsr;
            auto fdse = fdsr;

            timeval tv;
            tv.tv_sec = static_cast<long>(sec);
            tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);

            const auto ret = handle_EINTR([&] { return select(static_cast<int>(sock + 1), &fdsr, &fdsw, &fdse, &tv); });

            if (ret == 0) {
                return Error::ConnectionTimeout;
            }

            if (ret > 0 && (FD_ISSET(sock, &fdsr) || FD_ISSET(sock, &fdsw))) {
                auto error = 0;
                socklen_t len = sizeof(error);
                const auto res = getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len);
                const auto successful = res >= 0 && !error;
                return successful ? Error::Success : Error::Connection;
            }
            return Error::Connection;
        }

        inline auto is_socket_alive(const socket_t sock) -> bool {
            const auto val = select_read(sock, 0, 0);
            if (val == 0) {
                return true;
            }
            if (val < 0 && errno == EBADF) {
                return false;
            }
            char buf[1];
            return read_socket(sock, &buf[0], sizeof(buf), MSG_PEEK) > 0;
        }

        class SocketStream final : public Stream {
        public:
            SocketStream(socket_t sock,
                         time_t read_timeout_sec,
                         time_t read_timeout_usec,
                         time_t write_timeout_sec,
                         time_t write_timeout_usec,
                         time_t max_timeout_msec = 0,
                         std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::time_point::min());
            ~SocketStream() override;

            [[nodiscard]] auto is_readable() const -> bool override;
            [[nodiscard]] auto is_writable() const -> bool override;
            auto read(char* ptr, size_t size) -> ssize_t override;
            auto write(const char* ptr, size_t size) -> ssize_t override;
            auto get_remote_ip_and_port(std::string& ip, int& port) const -> void override;
            auto get_local_ip_and_port(std::string& ip, int& port) const -> void override;
            [[nodiscard]] auto socket() const -> socket_t override;
            [[nodiscard]] auto duration() const -> time_t override;

        private:
            socket_t sock_;
            time_t read_timeout_sec_;
            time_t read_timeout_usec_;
            time_t write_timeout_sec_;
            time_t write_timeout_usec_;
            time_t max_timeout_msec_;
            const std::chrono::time_point<std::chrono::steady_clock> start_time;

            std::vector<char> read_buff_;
            size_t read_buff_off_ = 0;
            size_t read_buff_content_size_ = 0;

            static constexpr size_t read_buff_size_ = 1024l * 4;
        };

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        class SSLSocketStream final : public Stream {
        public:
            SSLSocketStream(socket_t sock,
                            SSL* ssl,
                            time_t read_timeout_sec,
                            time_t read_timeout_usec,
                            time_t write_timeout_sec,
                            time_t write_timeout_usec,
                            time_t max_timeout_msec = 0,
                            std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::time_point::min());
            ~SSLSocketStream() override;

            [[nodiscard]] auto is_readable() const -> bool override;
            [[nodiscard]] auto is_writable() const -> bool override;
            auto read(char* ptr, size_t size) -> ssize_t override;
            auto write(const char* ptr, size_t size) -> ssize_t override;
            auto get_remote_ip_and_port(std::string& ip, int& port) const -> void override;
            auto get_local_ip_and_port(std::string& ip, int& port) const -> void override;
            [[nodiscard]] auto socket() const -> socket_t override;
            [[nodiscard]] auto duration() const -> time_t override;

        private:
            socket_t sock_;
            SSL* ssl_;
            time_t read_timeout_sec_;
            time_t read_timeout_usec_;
            time_t write_timeout_sec_;
            time_t write_timeout_usec_;
            time_t max_timeout_msec_;
            const std::chrono::time_point<std::chrono::steady_clock> start_time;
        };
#endif

        inline auto keep_alive(const std::atomic<socket_t>& svr_sock, const socket_t sock, const time_t keep_alive_timeout_sec) -> bool {
            using namespace std::chrono;

            const auto start = steady_clock::now();
            const auto timeout = std::chrono::seconds{
                keep_alive_timeout_sec
            };

            while (true) {
                if (svr_sock == INVALID_SOCKET) {
                    break;
                }

                char buf;
                const int val = recv(sock, &buf, 1, MSG_PEEK);

                if (val > 0) {
                    return true;
                }
                if (val == 0) {
                    break;
                }

                if (WSAGetLastError() == WSAEWOULDBLOCK) {
                    if (steady_clock::now() - start < timeout) {
                        std::this_thread::sleep_for(1ms);
                        continue;
                    }
                    break;
                }
                break;
            }
            return false;
        }

        template <typename T>
        auto process_server_socket_core(const std::atomic<socket_t>& svr_sock,
                                        const socket_t sock,
                                        const size_t keep_alive_max_count,
                                        const time_t keep_alive_timeout_sec,
                                        T callback) -> bool {
            assert(keep_alive_max_count > 0);
            auto ret = false;
            auto count = keep_alive_max_count;
            while (count > 0 && keep_alive(svr_sock, sock, keep_alive_timeout_sec)) {
                auto close_connection = count == 1;
                auto connection_closed = false;
                ret = callback(close_connection, connection_closed);
                if (!ret || connection_closed) {
                    break;
                }
                count--;
            }
            return ret;
        }

        template <typename T>
        auto process_server_socket(const std::atomic<socket_t>& svr_sock,
                                   socket_t sock,
                                   size_t keep_alive_max_count,
                                   time_t keep_alive_timeout_sec,
                                   const time_t read_timeout_sec,
                                   const time_t read_timeout_usec,
                                   const time_t write_timeout_sec,
                                   const time_t write_timeout_usec,
                                   T callback) -> bool {
            return process_server_socket_core(svr_sock,
                                              sock,
                                              keep_alive_max_count,
                                              keep_alive_timeout_sec,
                                              [&](bool close_connection, bool& connection_closed) {
                                                  SocketStream strm(sock, read_timeout_sec, read_timeout_usec, write_timeout_sec, write_timeout_usec);
                                                  return callback(strm, close_connection, connection_closed);
                                              });
        }

        inline auto process_client_socket(const socket_t sock,
                                          const time_t read_timeout_sec,
                                          const time_t read_timeout_usec,
                                          const time_t write_timeout_sec,
                                          const time_t write_timeout_usec,
                                          const time_t max_timeout_msec,
                                          const std::chrono::time_point<std::chrono::steady_clock> start_time,
                                          const std::function<bool(Stream&)>& callback) -> bool {
            SocketStream strm(sock, read_timeout_sec, read_timeout_usec, write_timeout_sec, write_timeout_usec, max_timeout_msec, start_time);
            return callback(strm);
        }

        inline auto shutdown_socket(socket_t sock) -> int {
#ifdef _WIN32
            return shutdown(sock, SD_BOTH);
#else
  return shutdown(sock, SHUT_RDWR);
#endif
        }

        inline auto escape_abstract_namespace_unix_domain(const std::string& s) -> std::string {
            if (s.size() > 1 && s[0] == '\0') {
                auto ret = s;
                ret[0] = '@';
                return ret;
            }
            return s;
        }

        inline auto unescape_abstract_namespace_unix_domain(const std::string& s) -> std::string {
            if (s.size() > 1 && s[0] == '@') {
                auto ret = s;
                ret[0] = '\0';
                return ret;
            }
            return s;
        }

        template <typename BindOrConnect>
        auto create_socket(const std::string& host,
                           const std::string& ip,
                           const int port,
                           const int address_family,
                           const int socket_flags,
                           const bool tcp_nodelay,
                           const bool ipv6_v6only,
                           const SocketOptions& socket_options,
                           BindOrConnect bind_or_connect) -> socket_t {
            // Get address info
            const char* node = nullptr;
            addrinfo hints;
            addrinfo* result;

            memset(&hints, 0, sizeof(addrinfo));
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_IP;

            if (!ip.empty()) {
                node = ip.c_str();
                // Ask getaddrinfo to convert IP in c-string to address
                hints.ai_family = AF_UNSPEC;
                hints.ai_flags = AI_NUMERICHOST;
            } else {
                if (!host.empty()) {
                    node = host.c_str();
                }
                hints.ai_family = address_family;
                hints.ai_flags = socket_flags;
            }

            const auto service = std::to_string(port);

            if (getaddrinfo(node, service.c_str(), &hints, &result)) {
                return INVALID_SOCKET;
            }
            auto se = scope_exit([&] { freeaddrinfo(result); });

            for (auto rp = result; rp; rp = rp->ai_next) {
                // Create a socket
                auto sock = WSASocketW(rp->ai_family, rp->ai_socktype, rp->ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
                /**
                 * Since the WSA_FLAG_NO_HANDLE_INHERIT is only supported on Windows 7 SP1
                 * and above the socket creation fails on older Windows Systems.
                 *
                 * Let's try to create a socket the old way in this case.
                 *
                 * Reference:
                 * https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsasocketa
                 *
                 * WSA_FLAG_NO_HANDLE_INHERIT:
                 * This flag is supported on Windows 7 with SP1, Windows Server 2008 R2 with
                 * SP1, and later
                 *
                 */
                if (sock == INVALID_SOCKET) {
                    sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                }
                if (sock == INVALID_SOCKET) {
                    continue;
                }


                if (tcp_nodelay) {
                    auto opt = 1;
                    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&opt), sizeof(opt));
                }

                if (rp->ai_family == AF_INET6) {
                    auto opt = ipv6_v6only ? 1 : 0;
                    setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&opt), sizeof(opt));
                }

                if (socket_options) {
                    socket_options(sock);
                }

                // bind or connect
                auto quit = false;
                if (bind_or_connect(sock, *rp, quit)) {
                    return sock;
                }

                close_socket(sock);

                if (quit) {
                    break;
                }
            }

            return INVALID_SOCKET;
        }

        inline auto set_nonblocking(const socket_t sock, const bool nonblocking) -> void {
            auto flags = nonblocking ? 1UL : 0UL;
            ioctlsocket(sock, FIONBIO, &flags);
        }

        inline auto is_connection_error() -> bool { return WSAGetLastError() != WSAEWOULDBLOCK; }

        inline auto bind_ip_address(const socket_t sock, const std::string& host) -> bool {
            addrinfo hints;
            addrinfo* result;

            memset(&hints, 0, sizeof(addrinfo));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = 0;

            if (getaddrinfo(host.c_str(), "0", &hints, &result)) {
                return false;
            }
            auto se = scope_exit([&] { freeaddrinfo(result); });

            auto ret = false;
            for (auto rp = result; rp; rp = rp->ai_next) {
                const auto& ai = *rp;
                if (!bind(sock, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen))) {
                    ret = true;
                    break;
                }
            }

            return ret;
        }

        inline auto create_client_socket(const std::string& host,
                                         const std::string& ip,
                                         const int port,
                                         const int address_family,
                                         const bool tcp_nodelay,
                                         const bool ipv6_v6only,
                                         const SocketOptions& socket_options,
                                         const time_t connection_timeout_sec,
                                         const time_t connection_timeout_usec,
                                         const time_t read_timeout_sec,
                                         const time_t read_timeout_usec,
                                         const time_t write_timeout_sec,
                                         const time_t write_timeout_usec,
                                         const std::string& intf,
                                         Error& error) -> socket_t {
            const auto sock = create_socket(host,
                                            ip,
                                            port,
                                            address_family,
                                            0,
                                            tcp_nodelay,
                                            ipv6_v6only,
                                            socket_options,
                                            [&](const socket_t sock2, const struct addrinfo& ai, bool& quit) -> bool {
                                                if (!intf.empty()) {}

                                                set_nonblocking(sock2, true);

                                                const auto ret = connect(sock2, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen));

                                                if (ret < 0) {
                                                    if (is_connection_error()) {
                                                        error = Error::Connection;
                                                        return false;
                                                    }
                                                    error = wait_until_socket_is_ready(sock2, connection_timeout_sec, connection_timeout_usec);
                                                    if (error != Error::Success) {
                                                        if (error == Error::ConnectionTimeout) {
                                                            quit = true;
                                                        }
                                                        return false;
                                                    }
                                                }

                                                set_nonblocking(sock2, false);

                                                {
                                                    const auto timeout = static_cast<uint32_t>(read_timeout_sec * 1000 + read_timeout_usec / 1000);
                                                    setsockopt(sock2,
                                                               SOL_SOCKET,
                                                               SO_RCVTIMEO,
                                                               reinterpret_cast<const char*>(&timeout),
                                                               sizeof(timeout));
                                                }
                                                {
                                                    const auto timeout = static_cast<uint32_t>(write_timeout_sec * 1000 + write_timeout_usec / 1000);
                                                    setsockopt(sock2,
                                                               SOL_SOCKET,
                                                               SO_SNDTIMEO,
                                                               reinterpret_cast<const char*>(&timeout),
                                                               sizeof(timeout));
                                                }

                                                error = Error::Success;
                                                return true;
                                            });

            if (sock != INVALID_SOCKET) {
                error = Error::Success;
            } else {
                if (error == Error::Success) {
                    error = Error::Connection;
                }
            }

            return sock;
        }

        inline auto get_ip_and_port(const sockaddr_storage& addr, const socklen_t addr_len, std::string& ip, int& port) -> bool {
            if (addr.ss_family == AF_INET) {
                port = ntohs(reinterpret_cast<const sockaddr_in*>(&addr)->sin_port);
            } else if (addr.ss_family == AF_INET6) {
                port = ntohs(reinterpret_cast<const sockaddr_in6*>(&addr)->sin6_port);
            } else {
                return false;
            }

            std::array<char, NI_MAXHOST> ipstr{};
            if (getnameinfo(reinterpret_cast<const sockaddr*>(&addr), addr_len, ipstr.data(), ipstr.size(), nullptr, 0, NI_NUMERICHOST)) {
                return false;
            }

            ip = ipstr.data();
            return true;
        }

        inline auto get_local_ip_and_port(const socket_t sock, std::string& ip, int& port) -> void {
            sockaddr_storage addr;
            socklen_t addr_len = sizeof(addr);
            if (!getsockname(sock, reinterpret_cast<sockaddr*>(&addr), &addr_len)) {
                get_ip_and_port(addr, addr_len, ip, port);
            }
        }

        inline auto get_remote_ip_and_port(const socket_t sock, std::string& ip, int& port) -> void {
            sockaddr_storage addr;
            socklen_t addr_len = sizeof(addr);

            if (!getpeername(sock, reinterpret_cast<sockaddr*>(&addr), &addr_len)) {
                get_ip_and_port(addr, addr_len, ip, port);
            }
        }

        constexpr auto str2tag_core(const char* s, const size_t l, const unsigned int h) -> unsigned int {
            return l == 0
                       ? h
                       : str2tag_core(s + 1,
                                      l - 1,
                                      // Unsets the 6 high bits of h, therefore no overflow happens
                                      (std::numeric_limits<unsigned int>::max)() >> 6 & h * 33 ^ static_cast<unsigned char>(*s));
        }

        inline auto str2tag(const std::string& s) -> unsigned int { return str2tag_core(s.data(), s.size(), 0); }

        namespace udl {
            constexpr auto operator""_t(const char* s, const size_t l) -> unsigned int { return str2tag_core(s, l, 0); }
        } // namespace udl

        inline auto find_content_type(const std::string& path,
                                      const std::map<std::string, std::string>& user_data,
                                      const std::string& default_content_type) -> std::string {
            const auto ext = file_extension(path);

            const auto it = user_data.find(ext);
            if (it != user_data.end()) {
                return it->second;
            }

            using udl::operator""_t;

            switch (str2tag(ext)) {
                default:
                    return default_content_type;

                case "css"_t:
                    return "text/css";
                case "csv"_t:
                    return "text/csv";
                case "htm"_t:
                case "html"_t:
                    return "text/html";
                case "js"_t:
                case "mjs"_t:
                    return "text/javascript";
                case "txt"_t:
                    return "text/plain";
                case "vtt"_t:
                    return "text/vtt";

                case "apng"_t:
                    return "image/apng";
                case "avif"_t:
                    return "image/avif";
                case "bmp"_t:
                    return "image/bmp";
                case "gif"_t:
                    return "image/gif";
                case "png"_t:
                    return "image/png";
                case "svg"_t:
                    return "image/svg+xml";
                case "webp"_t:
                    return "image/webp";
                case "ico"_t:
                    return "image/x-icon";
                case "tif"_t:
                    return "image/tiff";
                case "tiff"_t:
                    return "image/tiff";
                case "jpg"_t:
                case "jpeg"_t:
                    return "image/jpeg";

                case "mp4"_t:
                    return "video/mp4";
                case "mpeg"_t:
                    return "video/mpeg";
                case "webm"_t:
                    return "video/webm";

                case "mp3"_t:
                    return "audio/mp3";
                case "mpga"_t:
                    return "audio/mpeg";
                case "weba"_t:
                    return "audio/webm";
                case "wav"_t:
                    return "audio/wave";

                case "otf"_t:
                    return "font/otf";
                case "ttf"_t:
                    return "font/ttf";
                case "woff"_t:
                    return "font/woff";
                case "woff2"_t:
                    return "font/woff2";

                case "7z"_t:
                    return "application/x-7z-compressed";
                case "atom"_t:
                    return "application/atom+xml";
                case "pdf"_t:
                    return "application/pdf";
                case "json"_t:
                    return "application/json";
                case "rss"_t:
                    return "application/rss+xml";
                case "tar"_t:
                    return "application/x-tar";
                case "xht"_t:
                case "xhtml"_t:
                    return "application/xhtml+xml";
                case "xslt"_t:
                    return "application/xslt+xml";
                case "xml"_t:
                    return "application/xml";
                case "gz"_t:
                    return "application/gzip";
                case "zip"_t:
                    return "application/zip";
                case "wasm"_t:
                    return "application/wasm";
            }
        }

        inline auto can_compress_content_type(const std::string& content_type) -> bool {
            using udl::operator""_t;

            const auto tag = str2tag(content_type);

            switch (tag) {
                case "image/svg+xml"_t:
                case "application/javascript"_t:
                case "application/json"_t:
                case "application/xml"_t:
                case "application/protobuf"_t:
                case "application/xhtml+xml"_t:
                    return true;

                case "text/event-stream"_t:
                    return false;

                default:
                    return !content_type.rfind("text/", 0);
            }
        }

        inline auto encoding_type(const Request& req, const Response& res) -> EncodingType {
            auto ret = can_compress_content_type(res.get_header_value("Content-Type"));
            if (!ret) {
                return EncodingType::None;
            }

            const auto& s = req.get_header_value("Accept-Encoding");
            (void)s;

#ifdef CPPHTTPLIB_BROTLI_SUPPORT
            // TODO: 'Accept-Encoding' has br, not br;q=0
            ret = s.find("br") != std::string::npos;
            if (ret) {
                return EncodingType::Brotli;
            }
#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
            // TODO: 'Accept-Encoding' has gzip, not gzip;q=0
            ret = s.find("gzip") != std::string::npos;
            if (ret)
                return EncodingType::Gzip;
#endif

            return EncodingType::None;
        }

        inline auto nocompressor::compress(const char* data, const size_t data_length, bool /*last*/, const Callback callback) -> bool {
            if (!data_length) {
                return true;
            }
            return callback(data, data_length);
        }

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
        inline gzip_compressor::gzip_compressor() {
            std::memset(&strm_, 0, sizeof(strm_));
            strm_.zalloc = nullptr;
            strm_.zfree = nullptr;
            strm_.opaque = nullptr;

            is_valid_ = deflateInit2(&strm_, Z_BEST_COMPRESSION, Z_DEFLATED, 31, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) == Z_OK;
        }

        inline gzip_compressor::~gzip_compressor() { deflateEnd(&strm_); }

        inline auto gzip_compressor::compress(const char* data, size_t data_length, const bool last, const Callback callback) -> bool {
            assert(is_valid_);

            do {
                constexpr size_t max_avail_in = (std::numeric_limits<decltype(strm_.avail_in)>::max)();

                strm_.avail_in = static_cast<decltype(strm_.avail_in)>((std::min)(data_length, max_avail_in));
                strm_.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data));

                data_length -= strm_.avail_in;
                data += strm_.avail_in;

                const auto flush = last && data_length == 0 ? Z_FINISH : Z_NO_FLUSH;

                std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};
                do {
                    strm_.avail_out = static_cast<uInt>(buff.size());
                    strm_.next_out = reinterpret_cast<Bytef*>(buff.data());

                    const auto ret = deflate(&strm_, flush);
                    if (ret == Z_STREAM_ERROR)
                        return false;

                    if (!callback(buff.data(), buff.size() - strm_.avail_out))
                        return false;
                } while (strm_.avail_out == 0);

                assert(strm_.avail_in == 0);
            } while (data_length > 0);

            return true;
        }

        inline gzip_decompressor::gzip_decompressor() {
            std::memset(&strm_, 0, sizeof(strm_));
            strm_.zalloc = nullptr;
            strm_.zfree = nullptr;
            strm_.opaque = nullptr;

            // 15 is the value of wbits, which should be at the maximum possible value
            // to ensure that any gzip stream can be decoded. The offset of 32 specifies
            // that the stream type should be automatically detected either gzip or
            // deflate.
            is_valid_ = inflateInit2(&strm_, 32 + 15) == Z_OK;
        }

        inline gzip_decompressor::~gzip_decompressor() { inflateEnd(&strm_); }

        inline auto gzip_decompressor::is_valid() const -> bool { return is_valid_; }

        inline auto gzip_decompressor::decompress(const char* data, size_t data_length, const Callback callback) -> bool {
            assert(is_valid_);

            auto ret = Z_OK;

            do {
                constexpr size_t max_avail_in = (std::numeric_limits<decltype(strm_.avail_in)>::max)();

                strm_.avail_in = static_cast<decltype(strm_.avail_in)>((std::min)(data_length, max_avail_in));
                strm_.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data));

                data_length -= strm_.avail_in;
                data += strm_.avail_in;

                std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};
                while (strm_.avail_in > 0 && ret == Z_OK) {
                    strm_.avail_out = static_cast<uInt>(buff.size());
                    strm_.next_out = reinterpret_cast<Bytef*>(buff.data());

                    ret = inflate(&strm_, Z_NO_FLUSH);

                    assert(ret != Z_STREAM_ERROR);
                    switch (ret) {
                        case Z_NEED_DICT:
                        case Z_DATA_ERROR:
                        case Z_MEM_ERROR:
                            inflateEnd(&strm_);
                            return false;
                    }

                    if (!callback(buff.data(), buff.size() - strm_.avail_out))
                        return false;
                }

                if (ret != Z_OK && ret != Z_STREAM_END)
                    return false;
            } while (data_length > 0);

            return true;
        }
#endif
        inline auto has_header(const Headers& headers, const std::string& key) -> bool { return headers.contains(key); }

        inline auto get_header_value(const Headers& headers, const std::string& key, const char* def, const size_t id) -> const char* {
            const auto rng = headers.equal_range(key);
            auto it = rng.first;
            std::advance(it, static_cast<ssize_t>(id));
            if (it != rng.second) {
                return it->second.c_str();
            }
            return def;
        }

        template <typename T>
        auto parse_header(const char* beg, const char* end, T fn) -> bool {
            // Skip trailing spaces and tabs.
            while (beg < end && is_space_or_tab(end[-1])) {
                end--;
            }

            auto p = beg;
            while (p < end && *p != ':') {
                p++;
            }

            const auto name = std::string(beg, p);
            if (!fields::is_field_name(name)) {
                return false;
            }

            if (p == end) {
                return false;
            }

            const auto key_end = p;

            if (*p++ != ':') {
                return false;
            }

            while (p < end && is_space_or_tab(*p)) {
                p++;
            }

            if (p <= end) {
                const auto key_len = key_end - beg;
                if (!key_len) {
                    return false;
                }

                auto key = std::string(beg, key_end);
                // auto val = (case_ignore::equal(key, "Location") ||
                //             case_ignore::equal(key, "Referer"))
                //                ? std::string(p, end)
                //                : decode_url(std::string(p, end), false);
                auto val = std::string(p, end);

                if (!fields::is_field_value(val)) {
                    return false;
                }

                if (case_ignore::equal(key, "Location") || case_ignore::equal(key, "Referer")) {
                    fn(key, val);
                } else {
                    fn(key, decode_url(val, false));
                }

                return true;
            }

            return false;
        }

        inline auto read_headers(Stream& strm, Headers& headers) -> bool {
            constexpr auto bufsiz = 2048;
            char buf[bufsiz];
            stream_line_reader line_reader(strm, buf, bufsiz);

            for (;;) {
                if (!line_reader.getline()) {
                    return false;
                }

                // Check if the line ends with CRLF.
                auto line_terminator_len = 2;
                if (line_reader.end_with_crlf()) {
                    // Blank line indicates end of headers.
                    if (line_reader.size() == 2) {
                        break;
                    }
                } else {
#ifdef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
      // Blank line indicates end of headers.
      if (line_reader.size() == 1) { break; }
      line_terminator_len = 1;
#else
                    continue; // Skip invalid line.
#endif
                }

                if (line_reader.size() > CPPHTTPLIB_HEADER_MAX_LENGTH) {
                    return false;
                }

                // Exclude line terminator
                const auto end = line_reader.ptr() + line_reader.size() - line_terminator_len;

                if (!parse_header(line_reader.ptr(),
                                  end,
                                  [&](const std::string& key, const std::string& val) { headers.emplace(key, val); })) {
                    return false;
                }
            }

            return true;
        }

        inline auto read_content_with_length(Stream& strm,
                                             const uint64_t len,
                                             const Progress& progress,
                                             const ContentReceiverWithProgress& out) -> bool {
            char buf[CPPHTTPLIB_RECV_BUFSIZ];

            uint64_t r = 0;
            while (r < len) {
                auto read_len = len - r;
                const auto n = strm.read(buf, (std::min)(read_len, CPPHTTPLIB_RECV_BUFSIZ));
                if (n <= 0) {
                    return false;
                }

                if (!out(buf, static_cast<size_t>(n), r, len)) {
                    return false;
                }
                r += static_cast<uint64_t>(n);

                if (progress) {
                    if (!progress(r, len)) {
                        return false;
                    }
                }
            }

            return true;
        }

        inline auto skip_content_with_length(Stream& strm, const uint64_t len) -> void {
            char buf[CPPHTTPLIB_RECV_BUFSIZ];
            uint64_t r = 0;
            while (r < len) {
                auto read_len = len - r;
                const auto n = strm.read(buf, (std::min)(read_len, CPPHTTPLIB_RECV_BUFSIZ));
                if (n <= 0) {
                    return;
                }
                r += static_cast<uint64_t>(n);
            }
        }

        inline auto read_content_without_length(Stream& strm, const ContentReceiverWithProgress& out) -> bool {
            char buf[CPPHTTPLIB_RECV_BUFSIZ];
            uint64_t r = 0;
            for (;;) {
                const auto n = strm.read(buf, CPPHTTPLIB_RECV_BUFSIZ);
                if (n == 0) {
                    return true;
                }
                if (n < 0) {
                    return false;
                }

                if (!out(buf, static_cast<size_t>(n), r, 0)) {
                    return false;
                }
                r += static_cast<uint64_t>(n);
            }
        }

        template <typename T>
        auto read_content_chunked(Stream& strm, T& x, const ContentReceiverWithProgress& out) -> bool {
            constexpr auto bufsiz = 16;
            char buf[bufsiz];

            stream_line_reader line_reader(strm, buf, bufsiz);

            if (!line_reader.getline()) {
                return false;
            }

            while (true) {
                char* end_ptr;

                const unsigned long chunk_len = std::strtoul(line_reader.ptr(), &end_ptr, 16);

                if (end_ptr == line_reader.ptr()) {
                    return false;
                }
                if (chunk_len == ULONG_MAX) {
                    return false;
                }

                if (chunk_len == 0) {
                    break;
                }

                if (!read_content_with_length(strm, chunk_len, nullptr, out)) {
                    return false;
                }

                if (!line_reader.getline()) {
                    return false;
                }

                if (strcmp(line_reader.ptr(), "\r\n") != 0) {
                    return false;
                }

                if (!line_reader.getline()) {
                    return false;
                }

                assert(chunk_len == 0);
            }

            // NOTE: In RFC 9112, '7.1 Chunked Transfer Coding' mentiones "The chunked
            // transfer coding is complete when a chunk with a chunk-size of zero is
            // received, possibly followed by a trailer section, and finally terminated by
            // an empty line". https://www.rfc-editor.org/rfc/rfc9112.html#section-7.1
            //
            // In '7.1.3. Decoding Chunked', however, the pseudo-code in the section
            // does't care for the existence of the final CRLF. In other words, it seems
            // to be ok whether the final CRLF exists or not in the chunked data.
            // https://www.rfc-editor.org/rfc/rfc9112.html#section-7.1.3
            //
            // According to the reference code in RFC 9112, cpp-htpplib now allows
            // chuncked transfer coding data without the final CRLF.
            if (!line_reader.getline()) {
                return true;
            }

            while (strcmp(line_reader.ptr(), "\r\n") != 0) {
                if (line_reader.size() > CPPHTTPLIB_HEADER_MAX_LENGTH) {
                    return false;
                }

                // Exclude line terminator
                constexpr auto line_terminator_len = 2;
                auto end = line_reader.ptr() + line_reader.size() - line_terminator_len;

                parse_header(line_reader.ptr(),
                             end,
                             [&](const std::string& key, const std::string& val) { x.headers.emplace(key, val); });

                if (!line_reader.getline()) {
                    return false;
                }
            }

            return true;
        }

        inline auto is_chunked_transfer_encoding(const Headers& headers) -> bool {
            return case_ignore::equal(get_header_value(headers, "Transfer-Encoding", "", 0), "chunked");
        }

        template <typename T, typename U>
        auto prepare_content_receiver(T& x, int& status, const ContentReceiverWithProgress& receiver, const bool decompress, U callback) -> bool {
            if (decompress) {
                const std::string encoding = x.get_header_value("Content-Encoding");
                std::unique_ptr<decompressor> decompressor;

                if (encoding == "gzip" || encoding == "deflate") {
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
                    decompressor = detail::make_unique<gzip_decompressor>();
#else
      status = StatusCode::UnsupportedMediaType_415;
      return false;
#endif
                } else if (encoding.find("br") != std::string::npos) {
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
                    decompressor = detail::make_unique<brotli_decompressor>();
#else
                    status = UnsupportedMediaType_415;
                    return false;
#endif
                }

                if (decompressor) {
                    if (decompressor->is_valid()) {
                        ContentReceiverWithProgress out = [&](const char* buf, size_t n, const uint64_t off, const uint64_t len) {
                            return decompressor->decompress(buf,
                                                            n,
                                                            [&](const char* buf2, const size_t n2) { return receiver(buf2, n2, off, len); });
                        };
                        return callback(std::move(out));
                    }
                    status = InternalServerError_500;
                    return false;
                }
            }

            ContentReceiverWithProgress out = [&](const char* buf, const size_t n, const uint64_t off, const uint64_t len) {
                return receiver(buf, n, off, len);
            };
            return callback(std::move(out));
        }

        template <typename T>
        auto read_content(Stream& strm,
                          T& x,
                          size_t payload_max_length,
                          int& status,
                          Progress progress,
                          const ContentReceiverWithProgress& receiver,
                          bool decompress) -> bool {
            return prepare_content_receiver(x,
                                            status,
                                            std::move(receiver),
                                            decompress,
                                            [&](const ContentReceiverWithProgress& out) {
                                                auto ret = true;
                                                auto exceed_payload_max_length = false;

                                                if (is_chunked_transfer_encoding(x.headers)) {
                                                    ret = read_content_chunked(strm, x, out);
                                                } else if (!has_header(x.headers, "Content-Length")) {
                                                    ret = read_content_without_length(strm, out);
                                                } else {
                                                    auto is_invalid_value = false;
                                                    auto len = get_header_value_u64(x.headers,
                                                                                    "Content-Length",
                                                                                    (std::numeric_limits<uint64_t>::max)(),
                                                                                    0,
                                                                                    is_invalid_value);

                                                    if (is_invalid_value) {
                                                        ret = false;
                                                    } else if (len > payload_max_length) {
                                                        exceed_payload_max_length = true;
                                                        skip_content_with_length(strm, len);
                                                        ret = false;
                                                    } else if (len > 0) {
                                                        ret = read_content_with_length(strm, len, std::move(progress), out);
                                                    }
                                                }

                                                if (!ret) {
                                                    status = exceed_payload_max_length ? PayloadTooLarge_413 : BadRequest_400;
                                                }
                                                return ret;
                                            });
        }

        inline auto write_request_line(Stream& strm, const std::string& method, const std::string& path) -> ssize_t {
            std::string s = method;
            s += " ";
            s += path;
            s += " HTTP/1.1\r\n";
            return strm.write(s.data(), s.size());
        }

        inline auto write_response_line(Stream& strm, const int status) -> ssize_t {
            std::string s = "HTTP/1.1 ";
            s += std::to_string(status);
            s += " ";
            s += status_message(status);
            s += "\r\n";
            return strm.write(s.data(), s.size());
        }

        inline auto write_headers(Stream& strm, const Headers& headers) -> ssize_t {
            ssize_t write_len = 0;
            for (const auto& x : headers) {
                std::string s;
                s = x.first;
                s += ": ";
                s += x.second;
                s += "\r\n";

                const auto len = strm.write(s.data(), s.size());
                if (len < 0) {
                    return len;
                }
                write_len += len;
            }
            const auto len = strm.write("\r\n");
            if (len < 0) {
                return len;
            }
            write_len += len;
            return write_len;
        }

        inline auto write_data(Stream& strm, const char* d, const size_t l) -> bool {
            size_t offset = 0;
            while (offset < l) {
                const auto length = strm.write(d + offset, l - offset);
                if (length < 0) {
                    return false;
                }
                offset += static_cast<size_t>(length);
            }
            return true;
        }

        template <typename T>
        auto write_content(Stream& strm,
                           const ContentProvider& content_provider,
                           size_t offset,
                           const size_t length,
                           T is_shutting_down,
                           Error& error) -> bool {
            const size_t end_offset = offset + length;
            auto ok = true;
            DataSink data_sink;

            data_sink.write = [&](const char* d, const size_t l) -> bool {
                if (ok) {
                    if (strm.is_writable() && write_data(strm, d, l)) {
                        offset += l;
                    } else {
                        ok = false;
                    }
                }
                return ok;
            };

            data_sink.is_writable = [&]() -> bool { return strm.is_writable(); };

            while (offset < end_offset && !is_shutting_down()) {
                if (!strm.is_writable()) {
                    error = Error::Write;
                    return false;
                }
                if (!content_provider(offset, end_offset - offset, data_sink)) {
                    error = Error::Canceled;
                    return false;
                }
                if (!ok) {
                    error = Error::Write;
                    return false;
                }
            }

            error = Error::Success;
            return true;
        }

        template <typename T>
        auto write_content(Stream& strm, const ContentProvider& content_provider, size_t offset, size_t length, const T& is_shutting_down) -> bool {
            auto error = Error::Success;
            return write_content(strm, content_provider, offset, length, is_shutting_down, error);
        }

        template <typename T>
        auto write_content_without_length(Stream& strm, const ContentProvider& content_provider, const T& is_shutting_down) -> bool {
            size_t offset = 0;
            auto data_available = true;
            auto ok = true;
            DataSink data_sink;

            data_sink.write = [&](const char* d, const size_t l) -> bool {
                if (ok) {
                    offset += l;
                    if (!strm.is_writable() || !write_data(strm, d, l)) {
                        ok = false;
                    }
                }
                return ok;
            };

            data_sink.is_writable = [&]() -> bool { return strm.is_writable(); };

            data_sink.done = [&](void) { data_available = false; };

            while (data_available && !is_shutting_down()) {
                if (!strm.is_writable()) {
                    return false;
                }
                if (!content_provider(offset, 0, data_sink)) {
                    return false;
                }
                if (!ok) {
                    return false;
                }
            }
            return true;
        }

        template <typename T, typename U>
        auto write_content_chunked(Stream& strm,
                                   const ContentProvider& content_provider,
                                   const T& is_shutting_down,
                                   U& compressor,
                                   Error& error) -> bool {
            size_t offset = 0;
            auto data_available = true;
            auto ok = true;
            DataSink data_sink;

            data_sink.write = [&](const char* d, size_t l) -> bool {
                if (ok) {
                    data_available = l > 0;
                    offset += l;

                    std::string payload;
                    if (compressor.compress(d,
                                            l,
                                            false,
                                            [&](const char* data, const size_t data_len) {
                                                payload.append(data, data_len);
                                                return true;
                                            })) {
                        if (!payload.empty()) {
                            // Emit chunked response header and footer for each chunk
                            const auto chunk = from_i_to_hex(payload.size()) + "\r\n" + payload + "\r\n";
                            if (!strm.is_writable() || !write_data(strm, chunk.data(), chunk.size())) {
                                ok = false;
                            }
                        }
                    } else {
                        ok = false;
                    }
                }
                return ok;
            };

            data_sink.is_writable = [&]() -> bool { return strm.is_writable(); };

            auto done_with_trailer = [&](const Headers* trailer) {
                if (!ok) {
                    return;
                }

                data_available = false;

                std::string payload;
                if (!compressor.compress(nullptr,
                                         0,
                                         true,
                                         [&](const char* data, const size_t data_len) {
                                             payload.append(data, data_len);
                                             return true;
                                         })) {
                    ok = false;
                    return;
                }

                if (!payload.empty()) {
                    // Emit chunked response header and footer for each chunk
                    const auto chunk = from_i_to_hex(payload.size()) + "\r\n" + payload + "\r\n";
                    if (!strm.is_writable() || !write_data(strm, chunk.data(), chunk.size())) {
                        ok = false;
                        return;
                    }
                }

                static const std::string done_marker("0\r\n");
                if (!write_data(strm, done_marker.data(), done_marker.size())) {
                    ok = false;
                }

                // Trailer
                if (trailer) {
                    for (const auto& kv : *trailer) {
                        std::string field_line = kv.first + ": " + kv.second + "\r\n";
                        if (!write_data(strm, field_line.data(), field_line.size())) {
                            ok = false;
                        }
                    }
                }

                static const std::string crlf("\r\n");
                if (!write_data(strm, crlf.data(), crlf.size())) {
                    ok = false;
                }
            };

            data_sink.done = [&](void) { done_with_trailer(nullptr); };

            data_sink.done_with_trailer = [&](const Headers& trailer) { done_with_trailer(&trailer); };

            while (data_available && !is_shutting_down()) {
                if (!strm.is_writable()) {
                    error = Error::Write;
                    return false;
                }
                if (!content_provider(offset, 0, data_sink)) {
                    error = Error::Canceled;
                    return false;
                }
                if (!ok) {
                    error = Error::Write;
                    return false;
                }
            }

            error = Error::Success;
            return true;
        }

        template <typename T, typename U>
        auto write_content_chunked(Stream& strm, const ContentProvider& content_provider, const T& is_shutting_down, U& compressor) -> bool {
            auto error = Error::Success;
            return write_content_chunked(strm, content_provider, is_shutting_down, compressor, error);
        }

        template <typename T>
        auto redirect(T& cli, Request& req, Response& res, const std::string& path, const std::string& location, Error& error) -> bool {
            Request new_req = req;
            new_req.path = path;
            new_req.redirect_count_ -= 1;

            if (res.status == SeeOther_303 && req.method != "GET" && req.method != "HEAD") {
                new_req.method = "GET";
                new_req.body.clear();
                new_req.headers.clear();
            }

            Response new_res;

            auto ret = cli.send(new_req, new_res, error);
            if (ret) {
                req = new_req;
                res = new_res;

                if (res.location.empty()) {
                    res.location = location;
                }
            }
            return ret;
        }

        inline auto params_to_query_str(const Params& params) -> std::string {
            std::string query;

            for (auto it = params.begin(); it != params.end(); ++it) {
                if (it != params.begin()) {
                    query += "&";
                }
                query += it->first;
                query += "=";
                query += encode_query_param(it->second);
            }
            return query;
        }

        inline auto parse_query_text(const char* data, const std::size_t size, Params& params) -> void {
            std::set<std::string> cache;
            split(data,
                  data + size,
                  '&',
                  [&](const char* b, const char* e) {
                      std::string kv(b, e);
                      if (cache.contains(kv)) {
                          return;
                      }
                      cache.insert(std::move(kv));

                      std::string key;
                      std::string val;
                      divide(b,
                             static_cast<std::size_t>(e - b),
                             '=',
                             [&](const char* lhs_data, const std::size_t lhs_size, const char* rhs_data, const std::size_t rhs_size) {
                                 key.assign(lhs_data, lhs_size);
                                 val.assign(rhs_data, rhs_size);
                             });

                      if (!key.empty()) {
                          params.emplace(decode_url(key, true), decode_url(val, true));
                      }
                  });
        }

        inline auto parse_query_text(const std::string& s, Params& params) -> void { parse_query_text(s.data(), s.size(), params); }

        inline auto parse_multipart_boundary(const std::string& content_type, std::string& boundary) -> bool {
            const auto boundary_keyword = "boundary=";
            const auto pos = content_type.find(boundary_keyword);
            if (pos == std::string::npos) {
                return false;
            }
            const auto end = content_type.find(';', pos);
            const auto beg = pos + strlen(boundary_keyword);
            boundary = trim_double_quotes_copy(content_type.substr(beg, end - beg));
            return !boundary.empty();
        }

        inline auto parse_disposition_params(const std::string& s, Params& params) -> void {
            std::set<std::string> cache;
            split(s.data(),
                  s.data() + s.size(),
                  ';',
                  [&](const char* b, const char* e) {
                      const std::string kv(b, e);
                      if (cache.contains(kv)) {
                          return;
                      }
                      cache.insert(kv);

                      std::string key;
                      std::string val;
                      split(b,
                            e,
                            '=',
                            [&](const char* b2, const char* e2) {
                                if (key.empty()) {
                                    key.assign(b2, e2);
                                } else {
                                    val.assign(b2, e2);
                                }
                            });

                      if (!key.empty()) {
                          params.emplace(trim_double_quotes_copy(key), trim_double_quotes_copy(val));
                      }
                  });
        }

#ifdef CPPHTTPLIB_NO_EXCEPTIONS
inline bool parse_range_header(const std::string &s, Ranges &ranges) {
#else
        inline auto parse_range_header(const std::string& s, Ranges& ranges) -> bool try {
#endif
            auto is_valid = [](const std::string& str) {
                return std::all_of(str.cbegin(),
                                   str.cend(),
                                   [](const unsigned char c) { return std::isdigit(c); });
            };

            if (s.size() > 7 && s.compare(0, 6, "bytes=") == 0) {
                constexpr auto pos = static_cast<size_t>(6);
                const auto len = s.size() - 6;
                auto all_valid_ranges = true;
                split(&s[pos],
                      &s[pos + len],
                      ',',
                      [&](const char* b, const char* e) {
                          if (!all_valid_ranges) {
                              return;
                          }

                          const auto it = std::find(b, e, '-');
                          if (it == e) {
                              all_valid_ranges = false;
                              return;
                          }

                          const auto lhs = std::string(b, it);
                          const auto rhs = std::string(it + 1, e);
                          if (!is_valid(lhs) || !is_valid(rhs)) {
                              all_valid_ranges = false;
                              return;
                          }

                          const auto first = lhs.empty() ? -1 : std::stoll(lhs);
                          const auto last = rhs.empty() ? -1 : std::stoll(rhs);
                          if ((first == -1 && last == -1) || (first != -1 && last != -1 && first > last)) {
                              all_valid_ranges = false;
                              return;
                          }

                          ranges.emplace_back(first, last);
                      });
                return all_valid_ranges && !ranges.empty();
            }
            return false;
#ifdef CPPHTTPLIB_NO_EXCEPTIONS
}
#else
        } catch (...) { return false; }
#endif

        class MultipartFormDataParser {
        public:
            MultipartFormDataParser() = default;

            auto set_boundary(std::string&& boundary) -> void {
                boundary_ = boundary;
                dash_boundary_crlf_ = dash_ + boundary_ + crlf_;
                crlf_dash_boundary_ = crlf_ + dash_ + boundary_;
            }

            [[nodiscard]] auto is_valid() const -> bool { return is_valid_; }

            auto parse(const char* buf, size_t n, const ContentReceiver& content_callback, const MultipartContentHeader& header_callback) -> bool {
                buf_append(buf, n);

                while (buf_size() > 0) {
                    switch (state_) {
                        case 0: {
                            // Initial boundary
                            buf_erase(buf_find(dash_boundary_crlf_));
                            if (dash_boundary_crlf_.size() > buf_size()) {
                                return true;
                            }
                            if (!buf_start_with(dash_boundary_crlf_)) {
                                return false;
                            }
                            buf_erase(dash_boundary_crlf_.size());
                            state_ = 1;
                            break;
                        }
                        case 1: {
                            // New entry
                            clear_file_info();
                            state_ = 2;
                            break;
                        }
                        case 2: {
                            // Headers
                            auto pos = buf_find(crlf_);
                            if (pos > CPPHTTPLIB_HEADER_MAX_LENGTH) {
                                return false;
                            }
                            while (pos < buf_size()) {
                                // Empty line
                                if (pos == 0) {
                                    if (!header_callback(file_)) {
                                        is_valid_ = false;
                                        return false;
                                    }
                                    buf_erase(crlf_.size());
                                    state_ = 3;
                                    break;
                                }

                                const auto header = buf_head(pos);

                                if (!parse_header(header.data(),
                                                  header.data() + header.size(),
                                                  [&](const std::string&, const std::string&) {})) {
                                    is_valid_ = false;
                                    return false;
                                }

                                static const std::string header_content_type = "Content-Type:";

                                if (start_with_case_ignore(header, header_content_type)) {
                                    file_.content_type = trim_copy(header.substr(header_content_type.size()));
                                } else {
                                    static const std::regex re_content_disposition(R"~(^Content-Disposition:\s*form-data;\s*(.*)$)~",
                                                                                   std::regex_constants::icase);

                                    std::smatch m;
                                    if (std::regex_match(header, m, re_content_disposition)) {
                                        Params params;
                                        parse_disposition_params(m[1], params);

                                        auto it = params.find("name");
                                        if (it != params.end()) {
                                            file_.name = it->second;
                                        } else {
                                            is_valid_ = false;
                                            return false;
                                        }

                                        it = params.find("filename");
                                        if (it != params.end()) {
                                            file_.filename = it->second;
                                        }

                                        it = params.find("filename*");
                                        if (it != params.end()) {
                                            // Only allow UTF-8 enconnding...
                                            static const std::regex re_rfc5987_encoding(R"~(^UTF-8''(.+?)$)~", std::regex_constants::icase);

                                            std::smatch m2;
                                            if (std::regex_match(it->second, m2, re_rfc5987_encoding)) {
                                                file_.filename = decode_url(m2[1], false); // override...
                                            } else {
                                                is_valid_ = false;
                                                return false;
                                            }
                                        }
                                    }
                                }
                                buf_erase(pos + crlf_.size());
                                pos = buf_find(crlf_);
                            }
                            if (state_ != 3) {
                                return true;
                            }
                            break;
                        }
                        case 3: {
                            // Body
                            if (crlf_dash_boundary_.size() > buf_size()) {
                                return true;
                            }
                            const auto pos = buf_find(crlf_dash_boundary_);
                            if (pos < buf_size()) {
                                if (!content_callback(buf_data(), pos)) {
                                    is_valid_ = false;
                                    return false;
                                }
                                buf_erase(pos + crlf_dash_boundary_.size());
                                state_ = 4;
                            } else {
                                const auto len = buf_size() - crlf_dash_boundary_.size();
                                if (len > 0) {
                                    if (!content_callback(buf_data(), len)) {
                                        is_valid_ = false;
                                        return false;
                                    }
                                    buf_erase(len);
                                }
                                return true;
                            }
                            break;
                        }
                        case 4: {
                            // Boundary
                            if (crlf_.size() > buf_size()) {
                                return true;
                            }
                            if (buf_start_with(crlf_)) {
                                buf_erase(crlf_.size());
                                state_ = 1;
                            } else {
                                if (dash_.size() > buf_size()) {
                                    return true;
                                }
                                if (buf_start_with(dash_)) {
                                    buf_erase(dash_.size());
                                    is_valid_ = true;
                                    buf_erase(buf_size()); // Remove epilogue
                                } else {
                                    return true;
                                }
                            }
                            break;
                        }
                    }
                }

                return true;
            }

        private:
            auto clear_file_info() -> void {
                file_.name.clear();
                file_.filename.clear();
                file_.content_type.clear();
            }

            static auto start_with_case_ignore(const std::string& a, const std::string& b) -> bool {
                if (a.size() < b.size()) {
                    return false;
                }
                for (size_t i = 0; i < b.size(); i++) {
                    if (case_ignore::to_lower(a[i]) != case_ignore::to_lower(b[i])) {
                        return false;
                    }
                }
                return true;
            }

            const std::string dash_ = "--";
            const std::string crlf_ = "\r\n";
            std::string boundary_;
            std::string dash_boundary_crlf_;
            std::string crlf_dash_boundary_;

            size_t state_ = 0;
            bool is_valid_ = false;
            MultipartFormData file_;

            // Buffer
            static auto start_with(const std::string& a, const size_t spos, const size_t epos, const std::string& b) -> bool {
                if (epos - spos < b.size()) {
                    return false;
                }
                for (size_t i = 0; i < b.size(); i++) {
                    if (a[i + spos] != b[i]) {
                        return false;
                    }
                }
                return true;
            }

            [[nodiscard]] auto buf_size() const -> size_t { return buf_epos_ - buf_spos_; }

            [[nodiscard]] auto buf_data() const -> const char* { return &buf_[buf_spos_]; }

            [[nodiscard]] auto buf_head(const size_t l) const -> std::string { return buf_.substr(buf_spos_, l); }

            [[nodiscard]] auto buf_start_with(const std::string& s) const -> bool { return start_with(buf_, buf_spos_, buf_epos_, s); }

            [[nodiscard]] auto buf_find(const std::string& s) const -> size_t {
                const auto c = s.front();

                size_t off = buf_spos_;
                while (off < buf_epos_) {
                    auto pos = off;
                    while (true) {
                        if (pos == buf_epos_) {
                            return buf_size();
                        }
                        if (buf_[pos] == c) {
                            break;
                        }
                        pos++;
                    }

                    const auto remaining_size = buf_epos_ - pos;
                    if (s.size() > remaining_size) {
                        return buf_size();
                    }

                    if (start_with(buf_, pos, buf_epos_, s)) {
                        return pos - buf_spos_;
                    }

                    off = pos + 1;
                }

                return buf_size();
            }

            auto buf_append(const char* data, const size_t n) -> void {
                const auto remaining_size = buf_size();
                if (remaining_size > 0 && buf_spos_ > 0) {
                    for (size_t i = 0; i < remaining_size; i++) {
                        buf_[i] = buf_[buf_spos_ + i];
                    }
                }
                buf_spos_ = 0;
                buf_epos_ = remaining_size;

                if (remaining_size + n > buf_.size()) {
                    buf_.resize(remaining_size + n);
                }

                for (size_t i = 0; i < n; i++) {
                    buf_[buf_epos_ + i] = data[i];
                }
                buf_epos_ += n;
            }

            auto buf_erase(const size_t size) -> void { buf_spos_ += size; }

            std::string buf_;
            size_t buf_spos_ = 0;
            size_t buf_epos_ = 0;
        };

        inline auto random_string(const size_t length) -> std::string {
            static constexpr char data[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

            static auto engine([]() {
                // std::random_device might actually be deterministic on some
                // platforms, but due to lack of support in the c++ standard library,
                // doing better requires either some ugly hacks or breaking portability.
                std::random_device seed_gen;
                // Request 128 bits of entropy for initialization
                std::seed_seq seed_sequence{ seed_gen(), seed_gen(), seed_gen(), seed_gen() };
                return std::mt19937(seed_sequence);
            }());

            std::string result;
            for (size_t i = 0; i < length; i++) {
                result += data[engine() % (sizeof(data) - 1)];
            }
            return result;
        }

        inline auto make_multipart_data_boundary() -> std::string { return "--cpp-httplib-multipart-data-" + random_string(16); }

        inline auto is_multipart_boundary_chars_valid(const std::string& boundary) -> bool {
            auto valid = true;
            for (const char c : boundary) {
                if (!std::isalnum(c) && c != '-' && c != '_') {
                    valid = false;
                    break;
                }
            }
            return valid;
        }

        template <typename T>
        auto serialize_multipart_formdata_item_begin(const T& item, const std::string& boundary) -> std::string {
            std::string body = "--" + boundary + "\r\n";
            body += "Content-Disposition: form-data; name=\"" + item.name + "\"";
            if (!item.filename.empty()) {
                body += "; filename=\"" + item.filename + "\"";
            }
            body += "\r\n";
            if (!item.content_type.empty()) {
                body += "Content-Type: " + item.content_type + "\r\n";
            }
            body += "\r\n";

            return body;
        }

        inline auto serialize_multipart_formdata_item_end() -> std::string { return "\r\n"; }

        inline auto serialize_multipart_formdata_finish(const std::string& boundary) -> std::string { return "--" + boundary + "--\r\n"; }

        inline auto serialize_multipart_formdata_get_content_type(const std::string& boundary) -> std::string {
            return "multipart/form-data; boundary=" + boundary;
        }

        inline auto serialize_multipart_formdata(const MultipartFormDataItems& items,
                                                 const std::string& boundary,
                                                 const bool finish = true) -> std::string {
            std::string body;

            for (const auto& item : items) {
                body += serialize_multipart_formdata_item_begin(item, boundary);
                body += item.content + serialize_multipart_formdata_item_end();
            }

            if (finish) {
                body += serialize_multipart_formdata_finish(boundary);
            }

            return body;
        }

        inline auto range_error(Request& req, const Response& res) -> bool {
            if (!req.ranges.empty() && 200 <= res.status && res.status < 300) {
                const ssize_t contant_len = static_cast<ssize_t>(res.content_length_ ? res.content_length_ : res.body.size());

                ssize_t prev_first_pos = -1;
                ssize_t prev_last_pos = -1;
                size_t overwrapping_count = 0;

                // NOTE: The following Range check is based on '14.2. Range' in RFC 9110
                // 'HTTP Semantics' to avoid potential denial-of-service attacks.
                // https://www.rfc-editor.org/rfc/rfc9110#section-14.2

                // Too many ranges
                if (req.ranges.size() > CPPHTTPLIB_RANGE_MAX_COUNT) {
                    return true;
                }

                for (auto& r : req.ranges) {
                    auto& first_pos = r.first;
                    auto& last_pos = r.second;

                    if (first_pos == -1 && last_pos == -1) {
                        first_pos = 0;
                        last_pos = contant_len;
                    }

                    if (first_pos == -1) {
                        first_pos = contant_len - last_pos;
                        last_pos = contant_len - 1;
                    }

                    // NOTE: RFC-9110 '14.1.2. Byte Ranges':
                    // A client can limit the number of bytes requested without knowing the
                    // size of the selected representation. If the last-pos value is absent,
                    // or if the value is greater than or equal to the current length of the
                    // representation data, the byte range is interpreted as the remainder of
                    // the representation (i.e., the server replaces the value of last-pos
                    // with a value that is one less than the current length of the selected
                    // representation).
                    // https://www.rfc-editor.org/rfc/rfc9110.html#section-14.1.2-6
                    if (last_pos == -1 || last_pos >= contant_len) {
                        last_pos = contant_len - 1;
                    }

                    // Range must be within content length
                    if (!(0 <= first_pos && first_pos <= last_pos && last_pos <= contant_len - 1)) {
                        return true;
                    }

                    // Ranges must be in ascending order
                    if (first_pos <= prev_first_pos) {
                        return true;
                    }

                    // Request must not have more than two overlapping ranges
                    if (first_pos <= prev_last_pos) {
                        overwrapping_count++;
                        if (overwrapping_count > 2) {
                            return true;
                        }
                    }

                    prev_first_pos = (std::max)(prev_first_pos, first_pos);
                    prev_last_pos = (std::max)(prev_last_pos, last_pos);
                }
            }

            return false;
        }

        inline auto get_range_offset_and_length(Range r, const size_t content_length) -> std::pair<size_t, size_t> {
            assert(r.first != -1 && r.second != -1);
            assert(0 <= r.first && r.first < static_cast<ssize_t>(content_length));
            assert(r.first <= r.second && r.second < static_cast<ssize_t>(content_length));
            (void)content_length;
            return std::make_pair(r.first, static_cast<size_t>(r.second - r.first) + 1);
        }

        inline auto make_content_range_header_field(const std::pair<size_t, size_t>& offset_and_length, const size_t content_length) -> std::string {
            const auto st = offset_and_length.first;
            const auto ed = st + offset_and_length.second - 1;

            std::string field = "bytes ";
            field += std::to_string(st);
            field += "-";
            field += std::to_string(ed);
            field += "/";
            field += std::to_string(content_length);
            return field;
        }

        template <typename SToken, typename CToken, typename Content>
        auto process_multipart_ranges_data(const Request& req,
                                           const std::string& boundary,
                                           const std::string& content_type,
                                           const size_t content_length,
                                           SToken stoken,
                                           CToken ctoken,
                                           Content content) -> bool {
            for (const auto range : req.ranges) {
                ctoken("--");
                stoken(boundary);
                ctoken("\r\n");
                if (!content_type.empty()) {
                    ctoken("Content-Type: ");
                    stoken(content_type);
                    ctoken("\r\n");
                }

                auto offset_and_length = get_range_offset_and_length(range, content_length);

                ctoken("Content-Range: ");
                stoken(make_content_range_header_field(offset_and_length, content_length));
                ctoken("\r\n");
                ctoken("\r\n");

                if (!content(offset_and_length.first, offset_and_length.second)) {
                    return false;
                }
                ctoken("\r\n");
            }

            ctoken("--");
            stoken(boundary);
            ctoken("--");

            return true;
        }

        inline auto make_multipart_ranges_data(const Request& req,
                                               const Response& res,
                                               const std::string& boundary,
                                               const std::string& content_type,
                                               const size_t content_length,
                                               std::string& data) -> void {
            process_multipart_ranges_data(req,
                                          boundary,
                                          content_type,
                                          content_length,
                                          [&](const std::string& token) { data += token; },
                                          [&](const std::string& token) { data += token; },
                                          [&](const size_t offset, const size_t length) {
                                              assert(offset + length <= content_length);
                                              data += res.body.substr(offset, length);
                                              return true;
                                          });
        }

        inline auto get_multipart_ranges_data_length(const Request& req,
                                                     const std::string& boundary,
                                                     const std::string& content_type,
                                                     const size_t content_length) -> size_t {
            size_t data_length = 0;

            process_multipart_ranges_data(req,
                                          boundary,
                                          content_type,
                                          content_length,
                                          [&](const std::string& token) { data_length += token.size(); },
                                          [&](const std::string& token) { data_length += token.size(); },
                                          [&](size_t /*offset*/, const size_t length) {
                                              data_length += length;
                                              return true;
                                          });

            return data_length;
        }

        template <typename T>
        auto write_multipart_ranges_data(Stream& strm,
                                         const Request& req,
                                         Response& res,
                                         const std::string& boundary,
                                         const std::string& content_type,
                                         size_t content_length,
                                         const T& is_shutting_down) -> bool {
            return process_multipart_ranges_data(req,
                                                 boundary,
                                                 content_type,
                                                 content_length,
                                                 [&](const std::string& token) { strm.write(token); },
                                                 [&](const std::string& token) { strm.write(token); },
                                                 [&](size_t offset, size_t length) {
                                                     return write_content(strm, res.content_provider_, offset, length, is_shutting_down);
                                                 });
        }

        inline auto expect_content(const Request& req) -> bool {
            if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE") {
                return true;
            }
            if (req.has_header("Content-Length") && req.get_header_value_u64("Content-Length") > 0) {
                return true;
            }
            if (is_chunked_transfer_encoding(req.headers)) {
                return true;
            }
            return false;
        }

        inline auto has_crlf(const std::string& s) -> bool {
            auto p = s.c_str();
            while (*p) {
                if (*p == '\r' || *p == '\n') {
                    return true;
                }
                p++;
            }
            return false;
        }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        inline auto message_digest(const std::string& s, const EVP_MD* algo) -> std::string {
            const auto context = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(EVP_MD_CTX_new(), EVP_MD_CTX_free);

            unsigned int hash_length = 0;
            unsigned char hash[EVP_MAX_MD_SIZE];

            EVP_DigestInit_ex(context.get(), algo, nullptr);
            EVP_DigestUpdate(context.get(), s.c_str(), s.size());
            EVP_DigestFinal_ex(context.get(), hash, &hash_length);

            std::stringstream ss;
            for (auto i = 0u; i < hash_length; ++i)
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(hash[i]);

            return ss.str();
        }

        inline auto MD5(const std::string& s) -> std::string { return message_digest(s, EVP_md5()); }

        inline auto SHA_256(const std::string& s) -> std::string { return message_digest(s, EVP_sha256()); }

        inline auto SHA_512(const std::string& s) -> std::string { return message_digest(s, EVP_sha512()); }

        inline auto make_digest_authentication_header(const Request& req,
                                                      const std::map<std::string, std::string>& auth,
                                                      size_t cnonce_count,
                                                      const std::string& cnonce,
                                                      const std::string& username,
                                                      const std::string& password,
                                                      bool is_proxy = false) -> std::pair<std::string, std::string> {
            std::string nc;
            {
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(8) << std::hex << cnonce_count;
                nc = ss.str();
            }

            std::string qop;
            if (auth.contains("qop")) {
                qop = auth.at("qop");
                if (qop.find("auth-int") != std::string::npos)
                    qop = "auth-int";
                else if (qop.find("auth") != std::string::npos)
                    qop = "auth";
                else
                    qop.clear();
            }

            std::string algo = "MD5";
            if (auth.contains("algorithm"))
                algo = auth.at("algorithm");

            std::string response;
            {
                auto H = algo == "SHA-256" ? SHA_256 : algo == "SHA-512" ? SHA_512 : MD5;

                auto A1 = username + ":" + auth.at("realm") + ":" + password;

                auto A2 = req.method + ":" + req.path;
                if (qop == "auth-int")
                    A2 += ":" + H(req.body);

                if (qop.empty())
                    response = H(H(A1) + ":" + auth.at("nonce") + ":" + H(A2));
                else
                    response = H(H(A1) + ":" + auth.at("nonce") + ":" + nc + ":" + cnonce + ":" + qop + ":" + H(A2));
            }

            auto opaque = auth.contains("opaque") ? auth.at("opaque") : "";

            auto field = "Digest username=\"" + username + "\", realm=\"" + auth.at("realm") + "\", nonce=\"" + auth.at("nonce") + "\", uri=\"" + req.
                path + "\", algorithm=" + algo
                + (qop.empty() ? ", response=\"" : ", qop=" + qop + ", nc=" + nc + ", cnonce=\"" + cnonce + "\", response=\"") + response + "\"" + (
                    opaque.empty() ? "" : ", opaque=\"" + opaque + "\"");

            auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
            return std::make_pair(key, field);
        }

        inline auto is_ssl_peer_could_be_closed(SSL* ssl, const socket_t sock) -> bool {
            set_nonblocking(sock, true);
            auto se = scope_exit([&] { set_nonblocking(sock, false); });

            char buf[1];
            return !SSL_peek(ssl, buf, 1) && SSL_get_error(ssl, 0) == SSL_ERROR_ZERO_RETURN;
        }

#ifdef _WIN32
        // NOTE: This code came up with the following stackoverflow post:
        // https://stackoverflow.com/questions/9507184/can-openssl-on-windows-use-the-system-certificate-store
        inline auto load_system_certs_on_windows(X509_STORE* store) -> bool {
            const auto hStore = CertOpenSystemStoreW(NULL, L"ROOT");
            if (!hStore)
                return false;

            auto result = false;
            PCCERT_CONTEXT pContext = nullptr;
            while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != nullptr) {
                auto encoded_cert = static_cast<const unsigned char*>(pContext->pbCertEncoded);

                const auto x509 = d2i_X509(nullptr, &encoded_cert, pContext->cbCertEncoded);
                if (x509) {
                    X509_STORE_add_cert(store, x509);
                    X509_free(x509);
                    result = true;
                }
            }

            CertFreeCertificateContext(pContext);
            CertCloseStore(hStore, 0);

            return result;
        }
#elif defined(CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN) && defined(__APPLE__)
#if TARGET_OS_OSX
template <typename T>
using CFObjectPtr =
    std::unique_ptr<typename std::remove_pointer<T>::type, void (*)(CFTypeRef)>;

inline void cf_object_ptr_deleter(CFTypeRef obj) {
  if (obj) { CFRelease(obj); }
}

inline bool retrieve_certs_from_keychain(CFObjectPtr<CFArrayRef> &certs) {
  CFStringRef keys[] = {kSecClass, kSecMatchLimit, kSecReturnRef};
  CFTypeRef values[] = {kSecClassCertificate, kSecMatchLimitAll,
                        kCFBooleanTrue};

  CFObjectPtr<CFDictionaryRef> query(
      CFDictionaryCreate(nullptr, reinterpret_cast<const void **>(keys), values,
                         sizeof(keys) / sizeof(keys[0]),
                         &kCFTypeDictionaryKeyCallBacks,
                         &kCFTypeDictionaryValueCallBacks),
      cf_object_ptr_deleter);

  if (!query) { return false; }

  CFTypeRef security_items = nullptr;
  if (SecItemCopyMatching(query.get(), &security_items) != errSecSuccess ||
      CFArrayGetTypeID() != CFGetTypeID(security_items)) {
    return false;
  }

  certs.reset(reinterpret_cast<CFArrayRef>(security_items));
  return true;
}

inline bool retrieve_root_certs_from_keychain(CFObjectPtr<CFArrayRef> &certs) {
  CFArrayRef root_security_items = nullptr;
  if (SecTrustCopyAnchorCertificates(&root_security_items) != errSecSuccess) {
    return false;
  }

  certs.reset(root_security_items);
  return true;
}

inline bool add_certs_to_x509_store(CFArrayRef certs, X509_STORE *store) {
  auto result = false;
  for (auto i = 0; i < CFArrayGetCount(certs); ++i) {
    const auto cert = reinterpret_cast<const __SecCertificate *>(
        CFArrayGetValueAtIndex(certs, i));

    if (SecCertificateGetTypeID() != CFGetTypeID(cert)) { continue; }

    CFDataRef cert_data = nullptr;
    if (SecItemExport(cert, kSecFormatX509Cert, 0, nullptr, &cert_data) !=
        errSecSuccess) {
      continue;
    }

    CFObjectPtr<CFDataRef> cert_data_ptr(cert_data, cf_object_ptr_deleter);

    auto encoded_cert = static_cast<const unsigned char *>(
        CFDataGetBytePtr(cert_data_ptr.get()));

    auto x509 =
        d2i_X509(NULL, &encoded_cert, CFDataGetLength(cert_data_ptr.get()));

    if (x509) {
      X509_STORE_add_cert(store, x509);
      X509_free(x509);
      result = true;
    }
  }

  return result;
}

inline bool load_system_certs_on_macos(X509_STORE *store) {
  auto result = false;
  CFObjectPtr<CFArrayRef> certs(nullptr, cf_object_ptr_deleter);
  if (retrieve_certs_from_keychain(certs) && certs) {
    result = add_certs_to_x509_store(certs.get(), store);
  }

  if (retrieve_root_certs_from_keychain(certs) && certs) {
    result = add_certs_to_x509_store(certs.get(), store) || result;
  }

  return result;
}
#endif // TARGET_OS_OSX
#endif // _WIN32
#endif // CPPHTTPLIB_OPENSSL_SUPPORT

#ifdef _WIN32
        class WSInit {
        public:
            WSInit() {
                WSADATA wsaData;
                if (WSAStartup(0x0002, &wsaData) == 0) {
                    is_valid_ = true;
                }
            }

            ~WSInit() {
                if (is_valid_) {
                    WSACleanup();
                }
            }

            bool is_valid_ = false;
        };

        static WSInit wsinit_;
#endif

        inline auto parse_www_authenticate(const Response& res, std::map<std::string, std::string>& auth, bool is_proxy) -> bool {
            auto auth_key = is_proxy ? "Proxy-Authenticate" : "WWW-Authenticate";
            if (res.has_header(auth_key)) {
                static const auto re = std::regex(R"~((?:(?:,\s*)?(.+?)=(?:"(.*?)"|([^,]*))))~");
                auto s = res.get_header_value(auth_key);
                auto pos = s.find(' ');
                if (pos != std::string::npos) {
                    auto type = s.substr(0, pos);
                    if (type == "Basic") {
                        return false;
                    }
                    if (type == "Digest") {
                        s = s.substr(pos + 1);
                        auto beg = std::sregex_iterator(s.begin(), s.end(), re);
                        for (auto i = beg; i != std::sregex_iterator(); ++i) {
                            const auto& m = *i;
                            auto key = s.substr(static_cast<size_t>(m.position(1)), static_cast<size_t>(m.length(1)));
                            auto val = m.length(2) > 0
                                           ? s.substr(static_cast<size_t>(m.position(2)), static_cast<size_t>(m.length(2)))
                                           : s.substr(static_cast<size_t>(m.position(3)), static_cast<size_t>(m.length(3)));
                            auth[key] = val;
                        }
                        return true;
                    }
                }
            }
            return false;
        }

        class ContentProviderAdapter {
        public:
            explicit ContentProviderAdapter(ContentProviderWithoutLength&& content_provider) : content_provider_(content_provider) {}

            auto operator()(const size_t offset, size_t, DataSink& sink) const -> bool { return content_provider_(offset, sink); }

        private:
            ContentProviderWithoutLength content_provider_;
        };
    } // namespace detail

    inline auto hosted_at(const std::string& hostname) -> std::string {
        std::vector<std::string> addrs;
        hosted_at(hostname, addrs);
        if (addrs.empty()) {
            return {};
        }
        return addrs[0];
    }

    inline auto hosted_at(const std::string& hostname, std::vector<std::string>& addrs) -> void {
        addrinfo hints;
        addrinfo* result;

        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0;

        if (getaddrinfo(hostname.c_str(), nullptr, &hints, &result)) {
#if defined __linux__ && !defined __ANDROID__
    res_init();
#endif
            return;
        }
        auto se = detail::scope_exit([&] { freeaddrinfo(result); });

        for (auto rp = result; rp; rp = rp->ai_next) {
            const auto& addr = *reinterpret_cast<struct sockaddr_storage*>(rp->ai_addr);
            std::string ip;
            auto dummy = -1;
            if (detail::get_ip_and_port(addr, sizeof(sockaddr_storage), ip, dummy)) {
                addrs.push_back(ip);
            }
        }
    }

    inline auto append_query_params(const std::string& path, const Params& params) -> std::string {
        std::string path_with_query = path;
        static const std::regex re("[^?]+\\?.*");
        const auto delm = std::regex_match(path, re) ? '&' : '?';
        path_with_query += delm + detail::params_to_query_str(params);
        return path_with_query;
    }

    // Header utilities
    inline auto make_range_header(const Ranges& ranges) -> std::pair<std::string, std::string> {
        std::string field = "bytes=";
        auto i = 0;
        for (const auto& r : ranges) {
            if (i != 0) {
                field += ", ";
            }
            if (r.first != -1) {
                field += std::to_string(r.first);
            }
            field += '-';
            if (r.second != -1) {
                field += std::to_string(r.second);
            }
            i++;
        }
        return std::make_pair("Range", std::move(field));
    }

    inline auto make_basic_authentication_header(const std::string& username,
                                                 const std::string& password,
                                                 const bool is_proxy) -> std::pair<std::string, std::string> {
        auto field = "Basic " + detail::base64_encode(username + ":" + password);
        auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
        return std::make_pair(key, std::move(field));
    }

    inline auto make_bearer_token_authentication_header(const std::string& token,
                                                        const bool is_proxy = false) -> std::pair<std::string, std::string> {
        auto field = "Bearer " + token;
        auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
        return std::make_pair(key, std::move(field));
    }

    // Request implementation
    inline auto Request::has_header(const std::string& key) const -> bool { return detail::has_header(headers, key); }

    inline auto Request::get_header_value(const std::string& key, const char* def, const size_t id) const -> std::string {
        return detail::get_header_value(headers, key, def, id);
    }

    inline auto Request::get_header_value_count(const std::string& key) const -> size_t {
        const auto r = headers.equal_range(key);
        return static_cast<size_t>(std::distance(r.first, r.second));
    }

    inline auto Request::set_header(const std::string& key, const std::string& val) -> void {
        if (detail::fields::is_field_name(key) && detail::fields::is_field_value(val)) {
            headers.emplace(key, val);
        }
    }

    inline auto Request::has_param(const std::string& key) const -> bool { return params.contains(key); }

    inline auto Request::get_param_value(const std::string& key, const size_t id) const -> std::string {
        const auto rng = params.equal_range(key);
        auto it = rng.first;
        std::advance(it, static_cast<ssize_t>(id));
        if (it != rng.second) {
            return it->second;
        }
        return {};
    }

    inline auto Request::get_param_value_count(const std::string& key) const -> size_t {
        const auto r = params.equal_range(key);
        return static_cast<size_t>(std::distance(r.first, r.second));
    }

    inline auto Request::is_multipart_form_data() const -> bool {
        const auto& content_type = get_header_value("Content-Type");
        return !content_type.rfind("multipart/form-data", 0);
    }

    inline auto Request::has_file(const std::string& key) const -> bool { return files.contains(key); }

    inline auto Request::get_file_value(const std::string& key) const -> MultipartFormData {
        const auto it = files.find(key);
        if (it != files.end()) {
            return it->second;
        }
        return {};
    }

    inline auto Request::get_file_values(const std::string& key) const -> std::vector<MultipartFormData> {
        std::vector<MultipartFormData> values;
        const auto rng = files.equal_range(key);
        for (auto it = rng.first; it != rng.second; ++it) {
            values.push_back(it->second);
        }
        return values;
    }

    // Response implementation
    inline auto Response::has_header(const std::string& key) const -> bool { return headers.contains(key); }

    inline auto Response::get_header_value(const std::string& key, const char* def, const size_t id) const -> std::string {
        return detail::get_header_value(headers, key, def, id);
    }

    inline auto Response::get_header_value_count(const std::string& key) const -> size_t {
        const auto r = headers.equal_range(key);
        return static_cast<size_t>(std::distance(r.first, r.second));
    }

    inline auto Response::set_header(const std::string& key, const std::string& val) -> void {
        if (detail::fields::is_field_name(key) && detail::fields::is_field_value(val)) {
            headers.emplace(key, val);
        }
    }

    inline auto Response::set_redirect(const std::string& url, const int stat) -> void {
        if (detail::fields::is_field_value(url)) {
            set_header("Location", url);
            if (300 <= stat && stat < 400) {
                this->status = stat;
            } else {
                this->status = Found_302;
            }
        }
    }

    inline auto Response::set_content(const char* s, const size_t n, const std::string& content_type) -> void {
        body.assign(s, n);

        const auto rng = headers.equal_range("Content-Type");
        headers.erase(rng.first, rng.second);
        set_header("Content-Type", content_type);
    }

    inline auto Response::set_content(const std::string& s, const std::string& content_type) -> void {
        set_content(s.data(), s.size(), content_type);
    }

    inline auto Response::set_content(std::string&& s, const std::string& content_type) -> void {
        body = std::move(s);

        const auto rng = headers.equal_range("Content-Type");
        headers.erase(rng.first, rng.second);
        set_header("Content-Type", content_type);
    }

    inline auto Response::set_content_provider(const size_t in_length,
                                               const std::string& content_type,
                                               ContentProvider provider,
                                               ContentProviderResourceReleaser resource_releaser) -> void {
        set_header("Content-Type", content_type);
        content_length_ = in_length;
        if (in_length > 0) {
            content_provider_ = std::move(provider);
        }
        content_provider_resource_releaser_ = std::move(resource_releaser);
        is_chunked_content_provider_ = false;
    }

    inline auto Response::set_content_provider(const std::string& content_type,
                                               ContentProviderWithoutLength provider,
                                               ContentProviderResourceReleaser resource_releaser) -> void {
        set_header("Content-Type", content_type);
        content_length_ = 0;
        content_provider_ = detail::ContentProviderAdapter(std::move(provider));
        content_provider_resource_releaser_ = std::move(resource_releaser);
        is_chunked_content_provider_ = false;
    }

    inline auto Response::set_chunked_content_provider(const std::string& content_type,
                                                       ContentProviderWithoutLength provider,
                                                       ContentProviderResourceReleaser resource_releaser) -> void {
        set_header("Content-Type", content_type);
        content_length_ = 0;
        content_provider_ = detail::ContentProviderAdapter(std::move(provider));
        content_provider_resource_releaser_ = std::move(resource_releaser);
        is_chunked_content_provider_ = true;
    }

    inline auto Response::set_file_content(const std::string& path, const std::string& content_type) -> void {
        file_content_path_ = path;
        file_content_content_type_ = content_type;
    }

    inline auto Response::set_file_content(const std::string& path) -> void { file_content_path_ = path; }

    // Result implementation
    inline auto Result::has_request_header(const std::string& key) const -> bool { return request_headers_.contains(key); }

    inline auto Result::get_request_header_value(const std::string& key, const char* def, const size_t id) const -> std::string {
        return detail::get_header_value(request_headers_, key, def, id);
    }

    inline auto Result::get_request_header_value_count(const std::string& key) const -> size_t {
        const auto r = request_headers_.equal_range(key);
        return static_cast<size_t>(std::distance(r.first, r.second));
    }

    // Stream implementation
    inline auto Stream::write(const char* ptr) -> ssize_t { return write(ptr, strlen(ptr)); }

    inline auto Stream::write(const std::string& s) -> ssize_t { return write(s.data(), s.size()); }

    namespace detail {
        inline auto calc_actual_timeout(const time_t max_timeout_msec,
                                        const time_t duration_msec,
                                        const time_t timeout_sec,
                                        const time_t timeout_usec,
                                        time_t& actual_timeout_sec,
                                        time_t& actual_timeout_usec) -> void {
            const auto timeout_msec = timeout_sec * 1000 + timeout_usec / 1000;

            const auto actual_timeout_msec = std::min(max_timeout_msec - duration_msec, timeout_msec);

            actual_timeout_sec = actual_timeout_msec / 1000;
            actual_timeout_usec = actual_timeout_msec % 1000 * 1000;
        }

        // Socket stream implementation
        inline SocketStream::SocketStream(const socket_t sock,
                                          const time_t read_timeout_sec,
                                          const time_t read_timeout_usec,
                                          const time_t write_timeout_sec,
                                          const time_t write_timeout_usec,
                                          const time_t max_timeout_msec,
                                          const std::chrono::time_point<std::chrono::steady_clock> start_time) : sock_(sock),
            read_timeout_sec_(read_timeout_sec),
            read_timeout_usec_(read_timeout_usec),
            write_timeout_sec_(write_timeout_sec),
            write_timeout_usec_(write_timeout_usec),
            max_timeout_msec_(max_timeout_msec),
            start_time(start_time),
            read_buff_(read_buff_size_, 0) {}

        inline SocketStream::~SocketStream() = default;

        inline auto SocketStream::is_readable() const -> bool {
            if (max_timeout_msec_ <= 0) {
                return select_read(sock_, read_timeout_sec_, read_timeout_usec_) > 0;
            }

            time_t read_timeout_sec;
            time_t read_timeout_usec;
            calc_actual_timeout(max_timeout_msec_, duration(), read_timeout_sec_, read_timeout_usec_, read_timeout_sec, read_timeout_usec);

            return select_read(sock_, read_timeout_sec, read_timeout_usec) > 0;
        }

        inline auto SocketStream::is_writable() const -> bool {
            return select_write(sock_, write_timeout_sec_, write_timeout_usec_) > 0 && is_socket_alive(sock_);
        }

        inline auto SocketStream::read(char* ptr, size_t size) -> ssize_t {
#ifdef _WIN32
            size = (std::min)(size, static_cast<size_t>((std::numeric_limits<int>::max)()));
#else
  size = (std::min)(size,
                    static_cast<size_t>((std::numeric_limits<ssize_t>::max)()));
#endif

            if (read_buff_off_ < read_buff_content_size_) {
                const auto remaining_size = read_buff_content_size_ - read_buff_off_;
                if (size <= remaining_size) {
                    std::memcpy(ptr, read_buff_.data() + read_buff_off_, size);
                    read_buff_off_ += size;
                    return static_cast<ssize_t>(size);
                }
                std::memcpy(ptr, read_buff_.data() + read_buff_off_, remaining_size);
                read_buff_off_ += remaining_size;
                return static_cast<ssize_t>(remaining_size);
            }

            if (!is_readable()) {
                return -1;
            }

            read_buff_off_ = 0;
            read_buff_content_size_ = 0;

            if (size < read_buff_size_) {
                const auto n = read_socket(sock_, read_buff_.data(), read_buff_size_, CPPHTTPLIB_RECV_FLAGS);
                if (n <= 0) {
                    return n;
                }
                if (n <= static_cast<ssize_t>(size)) {
                    std::memcpy(ptr, read_buff_.data(), static_cast<size_t>(n));
                    return n;
                }
                std::memcpy(ptr, read_buff_.data(), size);
                read_buff_off_ = size;
                read_buff_content_size_ = static_cast<size_t>(n);
                return static_cast<ssize_t>(size);
            }
            return read_socket(sock_, ptr, size, CPPHTTPLIB_RECV_FLAGS);
        }

        inline auto SocketStream::write(const char* ptr, size_t size) -> ssize_t {
            if (!is_writable()) {
                return -1;
            }

#if defined(_WIN32) && !defined(_WIN64)
  size =
      (std::min)(size, static_cast<size_t>((std::numeric_limits<int>::max)()));
#endif

            return send_socket(sock_, ptr, size, CPPHTTPLIB_SEND_FLAGS);
        }

        inline auto SocketStream::get_remote_ip_and_port(std::string& ip, int& port) const -> void {
            return detail::get_remote_ip_and_port(sock_, ip, port);
        }

        inline auto SocketStream::get_local_ip_and_port(std::string& ip, int& port) const -> void {
            return detail::get_local_ip_and_port(sock_, ip, port);
        }

        inline auto SocketStream::socket() const -> socket_t { return sock_; }

        inline auto SocketStream::duration() const -> time_t {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();
        }

        // Buffer stream implementation
        inline auto BufferStream::is_readable() const -> bool { return true; }

        inline auto BufferStream::is_writable() const -> bool { return true; }

        inline auto BufferStream::read(char* ptr, size_t size) -> ssize_t {
#if defined(_MSC_VER) && _MSC_VER < 1910
  auto len_read = buffer._Copy_s(ptr, size, size, position);
#else
            auto len_read = buffer.copy(ptr, size, position);
#endif
            position += len_read;
            return static_cast<ssize_t>(len_read);
        }

        inline auto BufferStream::write(const char* ptr, const size_t size) -> ssize_t {
            buffer.append(ptr, size);
            return static_cast<ssize_t>(size);
        }

        inline auto BufferStream::get_remote_ip_and_port(std::string& /*ip*/, int& /*port*/) const -> void {}

        inline auto BufferStream::get_local_ip_and_port(std::string& /*ip*/, int& /*port*/) const -> void {}

        inline auto BufferStream::socket() const -> socket_t { return 0; }

        inline auto BufferStream::duration() const -> time_t { return 0; }

        inline auto BufferStream::get_buffer() const -> const std::string& { return buffer; }

        inline PathParamsMatcher::PathParamsMatcher(const std::string& pattern) {
            static constexpr char marker[] = "/:";

            // One past the last ending position of a path param substring
            std::size_t last_param_end = 0;

#ifndef CPPHTTPLIB_NO_EXCEPTIONS
            // Needed to ensure that parameter names are unique during matcher
            // construction
            // If exceptions are disabled, only last duplicate path
            // parameter will be set
            const std::unordered_set<std::string> param_name_set;
#endif

            while (true) {
                const auto marker_pos = pattern.find(marker, last_param_end == 0 ? last_param_end : last_param_end - 1);
                if (marker_pos == std::string::npos) {
                    break;
                }

                static_fragments_.push_back(pattern.substr(last_param_end, marker_pos - last_param_end + 1));

                const auto param_name_start = marker_pos + 2;

                auto sep_pos = pattern.find(separator, param_name_start);
                if (sep_pos == std::string::npos) {
                    sep_pos = pattern.length();
                }

                auto param_name = pattern.substr(param_name_start, sep_pos - param_name_start);

#ifndef CPPHTTPLIB_NO_EXCEPTIONS
                if (param_name_set.contains(param_name)) {
                    const std::string msg = "Encountered path parameter '" + param_name + "' multiple times in route pattern '" + pattern + "'.";
                    throw std::invalid_argument(msg);
                }
#endif

                param_names_.push_back(std::move(param_name));

                last_param_end = sep_pos + 1;
            }

            if (last_param_end < pattern.length()) {
                static_fragments_.push_back(pattern.substr(last_param_end));
            }

            static_total_length_ = 0;
            for (const auto& frag : static_fragments_) {
                static_total_length_ += frag.length();
            }
        }

        inline auto PathParamsMatcher::match(Request& request) const -> bool {
            request.matches = Match();
            request.path_params.clear();
            request.path_params.reserve(param_names_.size());

            const std::string& path = request.path; // 使用局部引用减少访问开销
            if (path.length() < static_total_length_) {
                return false; // 快速失败：路径过短
            }

            size_t starting_pos = 0;
            const size_t num_fragments = static_fragments_.size(); // 缓存循环次数
            const size_t param_count = param_names_.size(); // 缓存参数数量

            for (size_t i = 0; i < num_fragments; ++i) {
                const auto& fragment = static_fragments_[i];
                const size_t frag_len = fragment.length();

                // 检查剩余路径长度是否足够
                if (starting_pos + frag_len > path.length()) {
                    return false;
                }

                // 使用memcmp进行高效内存块比较
                if (std::memcmp(path.data() + starting_pos, fragment.data(), frag_len) != 0) {
                    return false;
                }

                starting_pos += frag_len;

                if (i >= param_count) {
                    continue; // 无对应参数需处理
                }

                // 使用memchr查找分隔符
                const size_t remaining_length = path.length() - starting_pos;
                const char* start_ptr = path.data() + starting_pos;
                const char* sep_ptr = static_cast<const char*>(memchr(start_ptr, separator, remaining_length));
                const size_t sep_pos = (sep_ptr != nullptr) ? (sep_ptr - path.data()) : path.length();

                // 提取参数值
                const auto& param_name = param_names_[i];
                request.path_params.emplace(param_name, path.substr(starting_pos, sep_pos - starting_pos));

                // 更新起始位置
                starting_pos = sep_pos + 1;
            }

            // 检查是否匹配完整路径
            return starting_pos >= path.length();
        }

        inline auto RegexMatcher::match(Request& request) const -> bool {
            request.path_params.clear();
            request.matches.resize(size_);
            return regex_.Match(request.path, 0, request.path.size(), RE2::ANCHOR_BOTH, request.matches.data(), size_);
        }
    } // namespace detail

    // HTTP server implementation
    inline Server::Server() : new_task_queue([] { return new ThreadPool(CPPHTTPLIB_THREAD_POOL_COUNT); }) {
#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif
    }

    inline Server::~Server() = default;

    inline auto Server::make_matcher(const std::string& pattern) -> std::unique_ptr<detail::MatcherBase> {
        if (pattern.find("/:") != std::string::npos) {
            return detail::make_unique<detail::PathParamsMatcher>(pattern);
        }
        return detail::make_unique<detail::RegexMatcher>(pattern);
    }

    inline auto Server::Get(const std::string& pattern, Handler handler) -> Server& {
        get_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::Post(const std::string& pattern, Handler handler) -> Server& {
        post_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::Post(const std::string& pattern, HandlerWithContentReader handler) -> Server& {
        post_handlers_for_content_reader_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::Put(const std::string& pattern, Handler handler) -> Server& {
        put_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::Put(const std::string& pattern, HandlerWithContentReader handler) -> Server& {
        put_handlers_for_content_reader_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::Patch(const std::string& pattern, Handler handler) -> Server& {
        patch_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::Patch(const std::string& pattern, HandlerWithContentReader handler) -> Server& {
        patch_handlers_for_content_reader_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::Delete(const std::string& pattern, Handler handler) -> Server& {
        delete_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::Delete(const std::string& pattern, HandlerWithContentReader handler) -> Server& {
        delete_handlers_for_content_reader_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::Options(const std::string& pattern, Handler handler) -> Server& {
        options_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
        return *this;
    }

    inline auto Server::set_base_dir(const std::string& dir, const std::string& mount_point) -> bool { return set_mount_point(mount_point, dir); }

    inline auto Server::set_mount_point(const std::string& mount_point, const std::string& dir, Headers headers) -> bool {
        const detail::FileStat stat(dir);
        if (stat.is_dir()) {
            const std::string mnt = !mount_point.empty() ? mount_point : "/";
            if (!mnt.empty() && mnt[0] == '/') {
                base_dirs_.push_back({
                    mnt,
                    dir,
                    std::move(headers)
                });
                return true;
            }
        }
        return false;
    }

    inline auto Server::remove_mount_point(const std::string& mount_point) -> bool {
        for (auto it = base_dirs_.begin(); it != base_dirs_.end(); ++it) {
            if (it->mount_point == mount_point) {
                base_dirs_.erase(it);
                return true;
            }
        }
        return false;
    }

    inline auto Server::set_file_extension_and_mimetype_mapping(const std::string& ext, const std::string& mime) -> Server& {
        file_extension_and_mimetype_map_[ext] = mime;
        return *this;
    }

    inline auto Server::set_default_file_mimetype(const std::string& mime) -> Server& {
        default_file_mimetype_ = mime;
        return *this;
    }

    inline auto Server::set_file_request_handler(Handler handler) -> Server& {
        file_request_handler_ = std::move(handler);
        return *this;
    }

    inline auto Server::set_error_handler_core(HandlerWithResponse handler, std::true_type) -> Server& {
        error_handler_ = std::move(handler);
        return *this;
    }

    inline auto Server::set_error_handler_core(const Handler& handler, std::false_type) -> Server& {
        error_handler_ = [handler](const Request& req, Response& res) {
            handler(req, res);
            return HandlerResponse::Handled;
        };
        return *this;
    }

    inline auto Server::set_exception_handler(ExceptionHandler handler) -> Server& {
        exception_handler_ = std::move(handler);
        return *this;
    }

    inline auto Server::set_pre_routing_handler(HandlerWithResponse handler) -> Server& {
        pre_routing_handler_ = std::move(handler);
        return *this;
    }

    inline auto Server::set_post_routing_handler(Handler handler) -> Server& {
        post_routing_handler_ = std::move(handler);
        return *this;
    }

    inline auto Server::set_logger(Logger logger) -> Server& {
        logger_ = std::move(logger);
        return *this;
    }

    inline auto Server::set_expect_100_continue_handler(Expect100ContinueHandler handler) -> Server& {
        expect_100_continue_handler_ = std::move(handler);
        return *this;
    }

    inline auto Server::set_address_family(const int family) -> Server& {
        address_family_ = family;
        return *this;
    }

    inline auto Server::set_tcp_nodelay(const bool on) -> Server& {
        tcp_nodelay_ = on;
        return *this;
    }

    inline auto Server::set_ipv6_v6only(const bool on) -> Server& {
        ipv6_v6only_ = on;
        return *this;
    }

    inline auto Server::set_socket_options(SocketOptions socket_options) -> Server& {
        socket_options_ = std::move(socket_options);
        return *this;
    }

    inline auto Server::set_default_headers(Headers headers) -> Server& {
        default_headers_ = std::move(headers);
        return *this;
    }

    inline auto Server::set_header_writer(const std::function<ssize_t(Stream&, Headers&)>& writer) -> Server& {
        header_writer_ = writer;
        return *this;
    }

    inline auto Server::set_keep_alive_max_count(const size_t count) -> Server& {
        keep_alive_max_count_ = count;
        return *this;
    }

    inline auto Server::set_keep_alive_timeout(const time_t sec) -> Server& {
        keep_alive_timeout_sec_ = sec;
        return *this;
    }

    inline auto Server::set_read_timeout(const time_t sec, const time_t usec) -> Server& {
        read_timeout_sec_ = sec;
        read_timeout_usec_ = usec;
        return *this;
    }

    inline auto Server::set_write_timeout(const time_t sec, const time_t usec) -> Server& {
        write_timeout_sec_ = sec;
        write_timeout_usec_ = usec;
        return *this;
    }

    inline auto Server::set_idle_interval(const time_t sec, const time_t usec) -> Server& {
        idle_interval_sec_ = sec;
        idle_interval_usec_ = usec;
        return *this;
    }

    inline auto Server::set_payload_max_length(const size_t length) -> Server& {
        payload_max_length_ = length;
        return *this;
    }

    inline auto Server::bind_to_port(const std::string& host, const int port, const int socket_flags) -> bool {
        const auto ret = bind_internal(host, port, socket_flags);
        if (ret == -1) {
            is_decommisioned = true;
        }
        return ret >= 0;
    }

    inline auto Server::bind_to_any_port(const std::string& host, const int socket_flags) -> int {
        const auto ret = bind_internal(host, 0, socket_flags);
        if (ret == -1) {
            is_decommisioned = true;
        }
        return ret;
    }

    inline auto Server::listen_after_bind() -> bool { return listen_internal(); }

    inline auto Server::listen(const std::string& host, const int port, const int socket_flags) -> bool {
        return bind_to_port(host, port, socket_flags) && listen_internal();
    }

    inline auto Server::is_running() const -> bool { return is_running_; }

    inline auto Server::wait_until_ready() const -> void {
        while (!is_running_ && !is_decommisioned) {
            std::this_thread::sleep_for(std::chrono::milliseconds{
                1
            });
        }
    }

    inline auto Server::stop() -> void {
        if (is_running_) {
            assert(svr_sock_ != INVALID_SOCKET);
            const std::atomic sock(svr_sock_.exchange(INVALID_SOCKET));
            detail::shutdown_socket(sock);
            detail::close_socket(sock);
        }
        is_decommisioned = false;
    }

    inline auto Server::decommission() -> void { is_decommisioned = true; }

    inline auto Server::parse_request_line(const char* s, Request& req) -> bool {
        auto len = strlen(s);
        if (len < 2 || s[len - 2] != '\r' || s[len - 1] != '\n') {
            return false;
        }
        len -= 2;

        {
            size_t count = 0;

            detail::split(s,
                          s + len,
                          ' ',
                          [&](const char* b, const char* e) {
                              switch (count) {
                                  case 0:
                                      req.method = std::string(b, e);
                                      break;
                                  case 1:
                                      req.target = std::string(b, e);
                                      break;
                                  case 2:
                                      req.version = std::string(b, e);
                                      break;
                                  default:
                                      break;
                              }
                              count++;
                          });

            if (count != 3) {
                return false;
            }
        }

        static const std::set<std::string> methods{
            "GET",
            "HEAD",
            "POST",
            "PUT",
            "DELETE",
            "CONNECT",
            "OPTIONS",
            "TRACE",
            "PATCH",
            "PRI"
        };

        if (!methods.contains(req.method)) {
            return false;
        }

        if (req.version != "HTTP/1.1" && req.version != "HTTP/1.0") {
            return false;
        }

        {
            // Skip URL fragment
            for (size_t i = 0; i < req.target.size(); i++) {
                if (req.target[i] == '#') {
                    req.target.erase(i);
                    break;
                }
            }

            detail::divide(req.target,
                           '?',
                           [&](const char* lhs_data, const std::size_t lhs_size, const char* rhs_data, const std::size_t rhs_size) {
                               req.path = detail::decode_url(std::string(lhs_data, lhs_size), false);
                               detail::parse_query_text(rhs_data, rhs_size, req.params);
                           });
        }

        return true;
    }

    inline auto Server::write_response(Stream& strm, const bool close_connection, Request& req, Response& res) const -> bool {
        // NOTE: `req.ranges` should be empty, otherwise it will be applied
        // incorrectly to the error content.
        req.ranges.clear();
        return write_response_core(strm, close_connection, req, res, false);
    }

    inline auto Server::write_response_with_content(Stream& strm, const bool close_connection, const Request& req, Response& res) const -> bool {
        return write_response_core(strm, close_connection, req, res, true);
    }

    inline auto Server::write_response_core(Stream& strm,
                                            const bool close_connection,
                                            const Request& req,
                                            Response& res,
                                            bool need_apply_ranges) const -> bool {
        assert(res.status != -1);

        if (400 <= res.status && error_handler_ && error_handler_(req, res) == HandlerResponse::Handled) {
            need_apply_ranges = true;
        }

        std::string content_type;
        std::string boundary;
        if (need_apply_ranges) {
            apply_ranges(req, res, content_type, boundary);
        }

        // Prepare additional headers
        if (close_connection || req.get_header_value("Connection") == "close") {
            res.set_header("Connection", "close");
        } else {
            std::string s = "timeout=";
            s += std::to_string(keep_alive_timeout_sec_);
            s += ", max=";
            s += std::to_string(keep_alive_max_count_);
            res.set_header("Keep-Alive", s);
        }

        if ((!res.body.empty() || res.content_length_ > 0 || res.content_provider_) && !res.has_header("Content-Type")) {
            res.set_header("Content-Type", "text/plain");
        }

        if (res.body.empty() && !res.content_length_ && !res.content_provider_ && !res.has_header("Content-Length")) {
            res.set_header("Content-Length", "0");
        }

        if (req.method == "HEAD" && !res.has_header("Accept-Ranges")) {
            res.set_header("Accept-Ranges", "bytes");
        }

        if (post_routing_handler_) {
            post_routing_handler_(req, res);
        }

        // Response line and headers
        {
            detail::BufferStream bstrm;
            if (!write_response_line(bstrm, res.status)) {
                return false;
            }
            if (!header_writer_(bstrm, res.headers)) {
                return false;
            }

            // Flush buffer
            auto& data = bstrm.get_buffer();
            detail::write_data(strm, data.data(), data.size());
        }

        // Body
        auto ret = true;
        if (req.method != "HEAD") {
            if (!res.body.empty()) {
                if (!detail::write_data(strm, res.body.data(), res.body.size())) {
                    ret = false;
                }
            } else if (res.content_provider_) {
                if (write_content_with_provider(strm, req, res, boundary, content_type)) {
                    res.content_provider_success_ = true;
                } else {
                    ret = false;
                }
            }
        }

        // Log
        if (logger_) {
            logger_(req, res);
        }

        return ret;
    }

    inline auto Server::write_content_with_provider(Stream& strm,
                                                    const Request& req,
                                                    Response& res,
                                                    const std::string& boundary,
                                                    const std::string& content_type) const -> bool {
        auto is_shutting_down = [this] { return this->svr_sock_ == INVALID_SOCKET; };

        if (res.content_length_ > 0) {
            if (req.ranges.empty()) {
                return detail::write_content(strm, res.content_provider_, 0, res.content_length_, is_shutting_down);
            }
            if (req.ranges.size() == 1) {
                const auto offset_and_length = detail::get_range_offset_and_length(req.ranges[0], res.content_length_);

                return detail::write_content(strm, res.content_provider_, offset_and_length.first, offset_and_length.second, is_shutting_down);
            }
            return detail::write_multipart_ranges_data(strm, req, res, boundary, content_type, res.content_length_, is_shutting_down);
        }
        if (res.is_chunked_content_provider_) {
            const auto type = detail::encoding_type(req, res);

            std::unique_ptr<detail::compressor> compressor;
            if (type == detail::EncodingType::Gzip) {
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
                compressor = detail::make_unique<detail::gzip_compressor>();
#endif
            } else if (type == detail::EncodingType::Brotli) {
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
                compressor = detail::make_unique<detail::brotli_compressor>();
#endif
            } else {
                compressor = detail::make_unique<detail::nocompressor>();
            }
            assert(compressor != nullptr);

            return write_content_chunked(strm, res.content_provider_, is_shutting_down, *compressor);
        }
        return detail::write_content_without_length(strm, res.content_provider_, is_shutting_down);
    }

    inline auto Server::read_content(Stream& strm, Request& req, Response& res) const -> bool {
        MultipartFormDataMap::iterator cur;
        auto file_count = 0;
        if (read_content_core(strm,
                              req,
                              res,
                              // Regular
                              [&](const char* buf, const size_t n) {
                                  if (req.body.size() + n > req.body.max_size()) {
                                      return false;
                                  }
                                  req.body.append(buf, n);
                                  return true;
                              },
                              // Multipart
                              [&](const MultipartFormData& file) {
                                  if (file_count++ == CPPHTTPLIB_MULTIPART_FORM_DATA_FILE_MAX_COUNT) {
                                      return false;
                                  }
                                  cur = req.files.emplace(file.name, file);
                                  return true;
                              },
                              [&](const char* buf, const size_t n) {
                                  auto& content = cur->second.content;
                                  if (content.size() + n > content.max_size()) {
                                      return false;
                                  }
                                  content.append(buf, n);
                                  return true;
                              })) {
            const auto& content_type = req.get_header_value("Content-Type");
            if (!content_type.find("application/x-www-form-urlencoded")) {
                if (req.body.size() > CPPHTTPLIB_FORM_URL_ENCODED_PAYLOAD_MAX_LENGTH) {
                    res.status = PayloadTooLarge_413; // NOTE: should be 414?
                    return false;
                }
                detail::parse_query_text(req.body, req.params);
            }
            return true;
        }
        return false;
    }

    inline auto Server::read_content_with_content_receiver(Stream& strm,
                                                           Request& req,
                                                           Response& res,
                                                           const ContentReceiver& receiver,
                                                           const MultipartContentHeader& multipart_header,
                                                           const ContentReceiver& multipart_receiver) const -> bool {
        return read_content_core(strm, req, res, receiver, multipart_header, multipart_receiver);
    }

    inline auto Server::read_content_core(Stream& strm,
                                          Request& req,
                                          Response& res,
                                          const ContentReceiver& receiver,
                                          const MultipartContentHeader& multipart_header,
                                          const ContentReceiver& multipart_receiver) const -> bool {
        detail::MultipartFormDataParser multipart_form_data_parser;
        ContentReceiverWithProgress out;

        if (req.is_multipart_form_data()) {
            const auto& content_type = req.get_header_value("Content-Type");
            std::string boundary;
            if (!detail::parse_multipart_boundary(content_type, boundary)) {
                res.status = BadRequest_400;
                return false;
            }

            multipart_form_data_parser.set_boundary(std::move(boundary));
            out = [&](const char* buf, const size_t n, uint64_t /*off*/, uint64_t /*len*/) {
                /* For debug
                size_t pos = 0;
                while (pos < n) {
                  auto read_size = (std::min)<size_t>(1, n - pos);
                  auto ret = multipart_form_data_parser.parse(
                      buf + pos, read_size, multipart_receiver, multipart_header);
                  if (!ret) { return false; }
                  pos += read_size;
                }
                return true;
                */
                return multipart_form_data_parser.parse(buf, n, multipart_receiver, multipart_header);
            };
        } else {
            out = [receiver](const char* buf, const size_t n, uint64_t /*off*/, uint64_t /*len*/) { return receiver(buf, n); };
        }

        if (req.method == "DELETE" && !req.has_header("Content-Length")) {
            return true;
        }

        if (!detail::read_content(strm, req, payload_max_length_, res.status, nullptr, out, true)) {
            return false;
        }

        if (req.is_multipart_form_data()) {
            if (!multipart_form_data_parser.is_valid()) {
                res.status = BadRequest_400;
                return false;
            }
        }

        return true;
    }

    inline auto Server::handle_file_request(const Request& req, Response& res, const bool head) const -> bool {
        for (const auto& entry : base_dirs_) {
            // Prefix match
            if (!req.path.compare(0, entry.mount_point.size(), entry.mount_point)) {
                std::string sub_path = "/" + req.path.substr(entry.mount_point.size());
                if (detail::is_valid_path(sub_path)) {
                    auto path = entry.base_dir + sub_path;
                    if (path.back() == '/') {
                        path += "index.html";
                    }

                    detail::FileStat stat(path);

                    if (stat.is_dir()) {
                        res.set_redirect(sub_path + "/", MovedPermanently_301);
                        return true;
                    }

                    if (stat.is_file()) {
                        for (const auto& kv : entry.headers) {
                            res.set_header(kv.first, kv.second);
                        }

                        auto mm = std::make_shared<detail::mmap>(path.c_str());
                        if (!mm->is_open()) {
                            return false;
                        }

                        res.set_content_provider(mm->size(),
                                                 detail::find_content_type(path, file_extension_and_mimetype_map_, default_file_mimetype_),
                                                 [mm](const size_t offset, const size_t length, const DataSink& sink) -> bool {
                                                     sink.write(mm->data() + offset, length);
                                                     return true;
                                                 });

                        if (!head && file_request_handler_) {
                            file_request_handler_(req, res);
                        }

                        return true;
                    }
                }
            }
        }
        return false;
    }

    inline auto Server::create_server_socket(const std::string& host,
                                             const int port,
                                             const int socket_flags,
                                             const SocketOptions& socket_options) const -> socket_t {
        return detail::create_socket(host,
                                     std::string(),
                                     port,
                                     address_family_,
                                     socket_flags,
                                     tcp_nodelay_,
                                     ipv6_v6only_,
                                     socket_options,
                                     [this](const socket_t sock, const addrinfo& ai, bool& /*quit*/) -> bool {
                                         if (bind(sock, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen))) {
                                             return false;
                                         }
                                         if (::listen(sock, CPPHTTPLIB_LISTEN_BACKLOG)) {
                                             return false;
                                         }
                                         return true;
                                     });
    }

    inline auto Server::bind_internal(const std::string& host, const int port, const int socket_flags) -> int {
        if (is_decommisioned) {
            return -1;
        }

        if (!is_valid()) {
            return -1;
        }

        svr_sock_ = create_server_socket(host, port, socket_flags, socket_options_);
        if (svr_sock_ == INVALID_SOCKET) {
            return -1;
        }

        detail::set_nonblocking(svr_sock_, true);

        if (port == 0) {
            sockaddr_storage addr;
            socklen_t addr_len = sizeof(addr);
            if (getsockname(svr_sock_, reinterpret_cast<sockaddr*>(&addr), &addr_len) == -1) {
                return -1;
            }
            if (addr.ss_family == AF_INET) {
                return ntohs(reinterpret_cast<sockaddr_in*>(&addr)->sin_port);
            }
            if (addr.ss_family == AF_INET6) {
                return ntohs(reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port);
            }
            return -1;
        }
        return port;
    }

    inline auto Server::listen_internal() -> bool {
        if (is_decommisioned) {
            return false;
        }

        constexpr auto ret = true;
        is_running_ = true;
        auto se = detail::scope_exit([&] { is_running_ = false; });

        const std::unique_ptr<TaskQueue> task_queue(new_task_queue());

        IOCPSocket::build(svr_sock_);
        IOCPSocket::task_works([&](SOCKET sock, const std::string&& ip_, int port) {
            if (!task_queue->enqueue([this, sock, ip = ip_, port] {
                if (sock == INVALID_SOCKET) {
                    if (errno == EMFILE) {
                        std::this_thread::sleep_for(1ms);
                        return;
                    }
                    if (errno == EINTR || errno == EAGAIN) {
                        return;
                    }
                    if (svr_sock_ != INVALID_SOCKET) {
                        IOCPSocket::close_client(svr_sock_);
                    }
                    return;
                }

                {
                    const auto timeout = static_cast<uint32_t>(read_timeout_sec_ * 1000 + read_timeout_usec_ / 1000);
                    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
                }
                {
                    const auto timeout = static_cast<uint32_t>(write_timeout_sec_ * 1000 + write_timeout_usec_ / 1000);
                    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
                }
                process_and_close_socket(sock, ip, port);
            })) {
                IOCPSocket::close_client(sock);
            }
        });
        IOCPSocket::post_accepts();
        task_queue->shutdown();

        is_decommisioned = !ret;
        return ret;
    }

    inline auto Server::routing(Request& req, Response& res, Stream& strm) const -> bool {
        if (pre_routing_handler_ && pre_routing_handler_(req, res) == HandlerResponse::Handled) {
            return true;
        }

        // File handler
        const auto is_head_request = req.method == "HEAD";
        if ((req.method == "GET" || is_head_request) && handle_file_request(req, res, is_head_request)) {
            return true;
        }

        if (detail::expect_content(req)) {
            // Content reader handler
            {
                const ContentReader reader([&](const ContentReceiver& receiver) {
                                               return read_content_with_content_receiver(strm, req, res, receiver, nullptr, nullptr);
                                           },
                                           [&](const MultipartContentHeader& header, const ContentReceiver& receiver) {
                                               return read_content_with_content_receiver(strm, req, res, nullptr, header, receiver);
                                           });

                if (req.method == "POST") {
                    if (dispatch_request_for_content_reader(req, res, reader, post_handlers_for_content_reader_)) {
                        return true;
                    }
                } else if (req.method == "PUT") {
                    if (dispatch_request_for_content_reader(req, res, reader, put_handlers_for_content_reader_)) {
                        return true;
                    }
                } else if (req.method == "PATCH") {
                    if (dispatch_request_for_content_reader(req, res, reader, patch_handlers_for_content_reader_)) {
                        return true;
                    }
                } else if (req.method == "DELETE") {
                    if (dispatch_request_for_content_reader(req, res, reader, delete_handlers_for_content_reader_)) {
                        return true;
                    }
                }
            }

            // Read content into `req.body`
            if (!read_content(strm, req, res)) {
                return false;
            }
        }

        // Regular handler
        if (req.method == "GET" || req.method == "HEAD") {
            return dispatch_request(req, res, get_handlers_);
        }
        if (req.method == "POST") {
            return dispatch_request(req, res, post_handlers_);
        }
        if (req.method == "PUT") {
            return dispatch_request(req, res, put_handlers_);
        }
        if (req.method == "DELETE") {
            return dispatch_request(req, res, delete_handlers_);
        }
        if (req.method == "OPTIONS") {
            return dispatch_request(req, res, options_handlers_);
        }
        if (req.method == "PATCH") {
            return dispatch_request(req, res, patch_handlers_);
        }

        res.status = BadRequest_400;
        return false;
    }

    inline auto Server::dispatch_request(Request& req, Response& res, const Handlers& handlers) -> bool {
        for (const auto& x : handlers) {
            const auto& matcher = x.first;
            const auto& handler = x.second;

            if (matcher->match(req)) {
                handler(req, res);
                return true;
            }
        }
        return false;
    }

    inline auto Server::apply_ranges(const Request& req, Response& res, std::string& content_type, std::string& boundary) -> void {
        if (req.ranges.size() > 1 && res.status == PartialContent_206) {
            const auto it = res.headers.find("Content-Type");
            if (it != res.headers.end()) {
                content_type = it->second;
                res.headers.erase(it);
            }

            boundary = detail::make_multipart_data_boundary();

            res.set_header("Content-Type", "multipart/byteranges; boundary=" + boundary);
        }

        const auto type = detail::encoding_type(req, res);

        if (res.body.empty()) {
            if (res.content_length_ > 0) {
                size_t length = 0;
                if (req.ranges.empty() || res.status != PartialContent_206) {
                    length = res.content_length_;
                } else if (req.ranges.size() == 1) {
                    const auto offset_and_length = detail::get_range_offset_and_length(req.ranges[0], res.content_length_);

                    length = offset_and_length.second;

                    const auto content_range = detail::make_content_range_header_field(offset_and_length, res.content_length_);
                    res.set_header("Content-Range", content_range);
                } else {
                    length = detail::get_multipart_ranges_data_length(req, boundary, content_type, res.content_length_);
                }
                res.set_header("Content-Length", std::to_string(length));
            } else {
                if (res.content_provider_) {
                    if (res.is_chunked_content_provider_) {
                        res.set_header("Transfer-Encoding", "chunked");
                        if (type == detail::EncodingType::Gzip) {
                            res.set_header("Content-Encoding", "gzip");
                        } else if (type == detail::EncodingType::Brotli) {
                            res.set_header("Content-Encoding", "br");
                        }
                    }
                }
            }
        } else {
            if (req.ranges.empty() || res.status != PartialContent_206) {} else if (req.ranges.size() == 1) {
                const auto offset_and_length = detail::get_range_offset_and_length(req.ranges[0], res.body.size());
                const auto offset = offset_and_length.first;
                const auto length = offset_and_length.second;

                const auto content_range = detail::make_content_range_header_field(offset_and_length, res.body.size());
                res.set_header("Content-Range", content_range);

                assert(offset + length <= res.body.size());
                res.body = res.body.substr(offset, length);
            } else {
                std::string data;
                detail::make_multipart_ranges_data(req, res, boundary, content_type, res.body.size(), data);
                res.body.swap(data);
            }

            if (type != detail::EncodingType::None) {
                std::unique_ptr<detail::compressor> compressor;
                std::string content_encoding;

                if (type == detail::EncodingType::Gzip) {
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
                    compressor = detail::make_unique<detail::gzip_compressor>();
                    content_encoding = "gzip";
#endif
                } else if (type == detail::EncodingType::Brotli) {
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
                    compressor = detail::make_unique<detail::brotli_compressor>();
                    content_encoding = "br";
#endif
                }

                if (compressor) {
                    std::string compressed;
                    if (compressor->compress(res.body.data(),
                                             res.body.size(),
                                             true,
                                             [&](const char* data, const size_t data_len) {
                                                 compressed.append(data, data_len);
                                                 return true;
                                             })) {
                        res.body.swap(compressed);
                        res.set_header("Content-Encoding", content_encoding);
                    }
                }
            }

            const auto length = std::to_string(res.body.size());
            res.set_header("Content-Length", length);
        }
    }

    inline auto Server::dispatch_request_for_content_reader(Request& req,
                                                            Response& res,
                                                            const ContentReader& content_reader,
                                                            const HandlersForContentReader& handlers) -> bool {
        for (const auto& x : handlers) {
            const auto& matcher = x.first;
            const auto& handler = x.second;

            if (matcher->match(req)) {
                handler(req, res, content_reader);
                return true;
            }
        }
        return false;
    }

    inline auto Server::process_request(Stream& strm,
                                        const std::string& remote_addr,
                                        int remote_port,
                                        const std::string& local_addr,
                                        int local_port,
                                        bool close_connection,
                                        bool& connection_closed,
                                        const std::function<void(Request&)>& setup_request) const -> bool {
        std::array<char, 2048> buf{};

        detail::stream_line_reader line_reader(strm, buf.data(), buf.size());

        // Connection has been closed on client
        if (!line_reader.getline()) {
            return false;
        }

        Request req;

        Response res;
        res.version = "HTTP/1.1";
        res.headers = default_headers_;

        // Request line and headers
        if (!parse_request_line(line_reader.ptr(), req) || !detail::read_headers(strm, req.headers)) {
            res.status = BadRequest_400;
            return write_response(strm, close_connection, req, res);
        }

        // Check if the request URI doesn't exceed the limit
        if (req.target.size() > CPPHTTPLIB_REQUEST_URI_MAX_LENGTH) {
            Headers dummy;
            detail::read_headers(strm, dummy);
            res.status = UriTooLong_414;
            return write_response(strm, close_connection, req, res);
        }

        if (req.get_header_value("Connection") == "close") {
            connection_closed = true;
        }

        if (req.version == "HTTP/1.0" && req.get_header_value("Connection") != "Keep-Alive") {
            connection_closed = true;
        }

        req.remote_addr = remote_addr;
        req.remote_port = remote_port;
        req.set_header("REMOTE_ADDR", req.remote_addr);
        req.set_header("REMOTE_PORT", std::to_string(req.remote_port));

        req.local_addr = local_addr;
        req.local_port = local_port;
        req.set_header("LOCAL_ADDR", req.local_addr);
        req.set_header("LOCAL_PORT", std::to_string(req.local_port));

        if (req.has_header("Range")) {
            const auto& range_header_value = req.get_header_value("Range");
            if (!detail::parse_range_header(range_header_value, req.ranges)) {
                res.status = RangeNotSatisfiable_416;
                return write_response(strm, close_connection, req, res);
            }
        }

        if (setup_request) {
            setup_request(req);
        }

        if (req.get_header_value("Expect") == "100-continue") {
            int status = Continue_100;
            if (expect_100_continue_handler_) {
                status = expect_100_continue_handler_(req, res);
            }
            switch (status) {
                case Continue_100:
                case ExpectationFailed_417:
                    detail::write_response_line(strm, status);
                    strm.write("\r\n");
                    break;
                default:
                    connection_closed = true;
                    return write_response(strm, true, req, res);
            }
        }

        // Setup `is_connection_closed` method
        req.is_connection_closed = [&] { return !detail::is_socket_alive(strm.socket()); };

        // Routing
        auto routed = false;
#ifdef CPPHTTPLIB_NO_EXCEPTIONS
  routed = routing(req, res, strm);
#else
        try { routed = routing(req, res, strm); } catch (std::exception& e) {
            if (exception_handler_) {
                auto ep = std::current_exception();
                exception_handler_(req, res, ep);
                routed = true;
            } else {
                res.status = InternalServerError_500;
                std::string val;
                auto s = e.what();
                for (size_t i = 0; s[i]; i++) {
                    switch (s[i]) {
                        case '\r':
                            val += "\\r";
                            break;
                        case '\n':
                            val += "\\n";
                            break;
                        default:
                            val += s[i];
                            break;
                    }
                }
                res.set_header("EXCEPTION_WHAT", val);
            }
        } catch (...) {
            if (exception_handler_) {
                auto ep = std::current_exception();
                exception_handler_(req, res, ep);
                routed = true;
            } else {
                res.status = InternalServerError_500;
                res.set_header("EXCEPTION_WHAT", "UNKNOWN");
            }
        }
#endif
        if (routed) {
            if (res.status == -1) {
                res.status = req.ranges.empty() ? OK_200 : PartialContent_206;
            }

            // Serve file content by using a content provider
            if (!res.file_content_path_.empty()) {
                const auto& path = res.file_content_path_;
                auto mm = std::make_shared<detail::mmap>(path.c_str());
                if (!mm->is_open()) {
                    res.body.clear();
                    res.content_length_ = 0;
                    res.content_provider_ = nullptr;
                    res.status = NotFound_404;
                    return write_response(strm, close_connection, req, res);
                }

                auto content_type = res.file_content_content_type_;
                if (content_type.empty()) {
                    content_type = detail::find_content_type(path, file_extension_and_mimetype_map_, default_file_mimetype_);
                }

                res.set_content_provider(mm->size(),
                                         content_type,
                                         [mm](const size_t offset, const size_t length, const DataSink& sink) -> bool {
                                             sink.write(mm->data() + offset, length);
                                             return true;
                                         });
            }

            if (detail::range_error(req, res)) {
                res.body.clear();
                res.content_length_ = 0;
                res.content_provider_ = nullptr;
                res.status = RangeNotSatisfiable_416;
                return write_response(strm, close_connection, req, res);
            }

            return write_response_with_content(strm, close_connection, req, res);
        }
        if (res.status == -1) {
            res.status = NotFound_404;
        }

        return write_response(strm, close_connection, req, res);
    }

    inline auto Server::is_valid() const -> bool { return true; }

    inline auto Server::process_and_close_socket(const socket_t sock, const std::string& ip, const int port) -> bool {
        std::string remote_addr;
        int remote_port = 0;
        if (!ip.empty()) {
            remote_addr = ip;
            remote_port = port;
        } else {
            detail::get_remote_ip_and_port(sock, remote_addr, remote_port);
        }

        std::string local_addr;
        int local_port = 0;
        detail::get_local_ip_and_port(sock, local_addr, local_port);

        const auto ret = detail::process_server_socket(svr_sock_,
                                                       sock,
                                                       keep_alive_max_count_,
                                                       keep_alive_timeout_sec_,
                                                       read_timeout_sec_,
                                                       read_timeout_usec_,
                                                       write_timeout_sec_,
                                                       write_timeout_usec_,
                                                       [&](Stream& strm, const bool close_connection, bool& connection_closed) {
                                                           return process_request(strm,
                                                                                  remote_addr,
                                                                                  remote_port,
                                                                                  local_addr,
                                                                                  local_port,
                                                                                  close_connection,
                                                                                  connection_closed,
                                                                                  nullptr);
                                                       });

        IOCPSocket::close_client(sock);
        return ret;
    }

    // HTTP client implementation
    inline ClientImpl::ClientImpl(const std::string& host) : ClientImpl(host, 80, std::string(), std::string()) {}

    inline ClientImpl::ClientImpl(const std::string& host, const int port) : ClientImpl(host, port, std::string(), std::string()) {}

    inline ClientImpl::ClientImpl(const std::string& host,
                                  const int port,
                                  std::string client_cert_path,
                                  std::string client_key_path) : host_(detail::escape_abstract_namespace_unix_domain(host)),
                                                                 port_(port),
                                                                 host_and_port_(adjust_host_string(host_) + ":" + std::to_string(port)),
                                                                 client_cert_path_(std::move(client_cert_path)),
                                                                 client_key_path_(std::move(client_key_path)) {}

    inline ClientImpl::~ClientImpl() {
        // Wait until all the requests in flight are handled.
        size_t retry_count = 10;
        while (retry_count-- > 0) {
            {
                std::lock_guard guard(socket_mutex_);
                if (socket_requests_in_flight_ == 0) {
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        }
        std::lock_guard guard(socket_mutex_);
        shutdown_socket(socket_);
        close_socket(socket_);
    }

    inline auto ClientImpl::is_valid() const -> bool { return true; }

    inline auto ClientImpl::copy_settings(const ClientImpl& rhs) -> void {
        client_cert_path_ = rhs.client_cert_path_;
        client_key_path_ = rhs.client_key_path_;
        connection_timeout_sec_ = rhs.connection_timeout_sec_;
        read_timeout_sec_ = rhs.read_timeout_sec_;
        read_timeout_usec_ = rhs.read_timeout_usec_;
        write_timeout_sec_ = rhs.write_timeout_sec_;
        write_timeout_usec_ = rhs.write_timeout_usec_;
        max_timeout_msec_ = rhs.max_timeout_msec_;
        basic_auth_username_ = rhs.basic_auth_username_;
        basic_auth_password_ = rhs.basic_auth_password_;
        bearer_token_auth_token_ = rhs.bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        digest_auth_username_ = rhs.digest_auth_username_;
        digest_auth_password_ = rhs.digest_auth_password_;
#endif
        keep_alive_ = rhs.keep_alive_;
        follow_location_ = rhs.follow_location_;
        url_encode_ = rhs.url_encode_;
        address_family_ = rhs.address_family_;
        tcp_nodelay_ = rhs.tcp_nodelay_;
        ipv6_v6only_ = rhs.ipv6_v6only_;
        socket_options_ = rhs.socket_options_;
        compress_ = rhs.compress_;
        decompress_ = rhs.decompress_;
        interface_ = rhs.interface_;
        proxy_host_ = rhs.proxy_host_;
        proxy_port_ = rhs.proxy_port_;
        proxy_basic_auth_username_ = rhs.proxy_basic_auth_username_;
        proxy_basic_auth_password_ = rhs.proxy_basic_auth_password_;
        proxy_bearer_token_auth_token_ = rhs.proxy_bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        proxy_digest_auth_username_ = rhs.proxy_digest_auth_username_;
        proxy_digest_auth_password_ = rhs.proxy_digest_auth_password_;
#endif
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        ca_cert_file_path_ = rhs.ca_cert_file_path_;
        ca_cert_dir_path_ = rhs.ca_cert_dir_path_;
        ca_cert_store_ = rhs.ca_cert_store_;
#endif
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        server_certificate_verification_ = rhs.server_certificate_verification_;
        server_hostname_verification_ = rhs.server_hostname_verification_;
        server_certificate_verifier_ = rhs.server_certificate_verifier_;
#endif
        logger_ = rhs.logger_;
    }

    inline auto ClientImpl::create_client_socket(Error& error) const -> socket_t {
        if (!proxy_host_.empty() && proxy_port_ != -1) {
            return detail::create_client_socket(proxy_host_,
                                                std::string(),
                                                proxy_port_,
                                                address_family_,
                                                tcp_nodelay_,
                                                ipv6_v6only_,
                                                socket_options_,
                                                connection_timeout_sec_,
                                                connection_timeout_usec_,
                                                read_timeout_sec_,
                                                read_timeout_usec_,
                                                write_timeout_sec_,
                                                write_timeout_usec_,
                                                interface_,
                                                error);
        }

        // Check is custom IP specified for host_
        std::string ip;
        const auto it = addr_map_.find(host_);
        if (it != addr_map_.end()) {
            ip = it->second;
        }

        return detail::create_client_socket(host_,
                                            ip,
                                            port_,
                                            address_family_,
                                            tcp_nodelay_,
                                            ipv6_v6only_,
                                            socket_options_,
                                            connection_timeout_sec_,
                                            connection_timeout_usec_,
                                            read_timeout_sec_,
                                            read_timeout_usec_,
                                            write_timeout_sec_,
                                            write_timeout_usec_,
                                            interface_,
                                            error);
    }

    inline auto ClientImpl::create_and_connect_socket(Socket& socket, Error& error) -> bool {
        const auto sock = create_client_socket(error);
        if (sock == INVALID_SOCKET) {
            return false;
        }
        socket.sock = sock;
        return true;
    }

    inline auto ClientImpl::shutdown_ssl(Socket& /*socket*/, bool /*shutdown_gracefully*/) -> void {
        // If there are any requests in flight from threads other than us, then it's
        // a thread-unsafe race because individual ssl* objects are not thread-safe.
        assert(socket_requests_in_flight_ == 0 || socket_requests_are_from_thread_ == std::this_thread::get_id());
    }

    inline auto ClientImpl::shutdown_socket(const Socket& socket) -> void {
        if (socket.sock == INVALID_SOCKET) {
            return;
        }
        detail::shutdown_socket(socket.sock);
    }

    inline auto ClientImpl::close_socket(Socket& socket) const -> void {
        // If there are requests in flight in another thread, usually closing
        // the socket will be fine and they will simply receive an error when
        // using the closed socket, but it is still a bug since rarely the OS
        // may reassign the socket id to be used for a new socket, and then
        // suddenly they will be operating on a live socket that is different
        // than the one they intended!
        assert(this->socket_requests_in_flight_ == 0 || this->socket_requests_are_from_thread_ == std::this_thread::get_id());

        // It is also a bug if this happens while SSL is still active
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        assert(socket.ssl == nullptr);
#endif
        if (socket.sock == INVALID_SOCKET) {
            return;
        }
        detail::close_socket(socket.sock);
        socket.sock = INVALID_SOCKET;
    }

    inline auto ClientImpl::read_response_line(Stream& strm, const Request& req, Response& res) -> bool {
        std::array<char, 2048> buf{};

        detail::stream_line_reader line_reader(strm, buf.data(), buf.size());

        if (!line_reader.getline()) {
            return false;
        }

#ifdef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
        static const std::regex re("(HTTP/1\\.[01]) (\\d{3})(?: (.*?))?\r?\n");
#else
        static const  std::regex re("(HTTP/1\\.[01]) (\\d{3})(?: (.*?))?\r\n");
#endif

        std::cmatch m;
        if (!std::regex_match(line_reader.ptr(), m, re)) {
            return req.method == "CONNECT";
        }
        res.version = std::string(m[1]);
        res.status = std::stoi(std::string(m[2]));
        res.reason = std::string(m[3]);

        // Ignore '100 Continue'
        while (res.status == Continue_100) {
            if (!line_reader.getline()) {
                return false;
            }
            // CRLF
            if (!line_reader.getline()) {
                return false;
            }
            // next response line

            if (!std::regex_match(line_reader.ptr(), m, re)) {
                return false;
            }
            res.version = std::string(m[1]);
            res.status = std::stoi(std::string(m[2]));
            res.reason = std::string(m[3]);
        }

        return true;
    }

    inline auto ClientImpl::send(Request& req, Response& res, Error& error) -> bool {
        std::lock_guard request_mutex_guard(request_mutex_);
        auto ret = send_(req, res, error);
        if (error == Error::SSLPeerCouldBeClosed_) {
            assert(!ret);
            ret = send_(req, res, error);
        }
        return ret;
    }

    inline auto ClientImpl::send_(Request& req, Response& res, Error& error) -> bool {
        {
            std::lock_guard guard(socket_mutex_);

            // Set this to false immediately - if it ever gets set to true by the end of
            // the request, we know another thread instructed us to close the socket.
            socket_should_be_closed_when_request_is_done_ = false;

            auto is_alive = false;
            if (socket_.is_open()) {
                is_alive = detail::is_socket_alive(socket_.sock);

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
                if (is_alive && is_ssl()) {
                    if (detail::is_ssl_peer_could_be_closed(socket_.ssl, socket_.sock))
                        is_alive = false;
                }
#endif

                if (!is_alive) {
                    // Attempt to avoid sigpipe by shutting down nongracefully if it seems
                    // like the other side has already closed the connection Also, there
                    // cannot be any requests in flight from other threads since we locked
                    // request_mutex_, so safe to close everything immediately
                    constexpr bool shutdown_gracefully = false;
                    shutdown_ssl(socket_, shutdown_gracefully);
                    shutdown_socket(socket_);
                    close_socket(socket_);
                }
            }

            if (!is_alive) {
                if (!create_and_connect_socket(socket_, error)) {
                    return false;
                }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
                // TODO: refactoring
                if (is_ssl()) {
                    auto& scli = dynamic_cast<SSLClient&>(*this);
                    if (!proxy_host_.empty() && proxy_port_ != -1) {
                        auto success = false;
                        if (!scli.connect_with_proxy(socket_, req.start_time_, res, success, error))
                            return success;
                    }

                    if (!scli.initialize_ssl(socket_, error))
                        return false;
                }
#endif
            }

            // Mark the current socket as being in use so that it cannot be closed by
            // anyone else while this request is ongoing, even though we will be
            // releasing the mutex.
            if (socket_requests_in_flight_ > 1) {
                assert(socket_requests_are_from_thread_ == std::this_thread::get_id());
            }
            socket_requests_in_flight_ += 1;
            socket_requests_are_from_thread_ = std::this_thread::get_id();
        }

        for (const auto& header : default_headers_) {
            if (!req.headers.contains(header.first)) {
                req.headers.insert(header);
            }
        }

        auto ret = false;
        const auto close_connection = !keep_alive_;

        auto se = detail::scope_exit([&] {
            // Briefly lock mutex in order to mark that a request is no longer ongoing
            std::lock_guard guard(socket_mutex_);
            socket_requests_in_flight_ -= 1;
            if (socket_requests_in_flight_ <= 0) {
                assert(socket_requests_in_flight_ == 0);
                socket_requests_are_from_thread_ = std::thread::id();
            }

            if (socket_should_be_closed_when_request_is_done_ || close_connection || !ret) {
                shutdown_ssl(socket_, true);
                shutdown_socket(socket_);
                close_socket(socket_);
            }
        });

        ret = process_socket(socket_,
                             req.start_time_,
                             [&](Stream& strm) { return handle_request(strm, req, res, close_connection, error); });

        if (!ret) {
            if (error == Error::Success) {
                error = Error::Unknown;
            }
        }

        return ret;
    }

    inline auto ClientImpl::send(const Request& req) -> Result {
        auto req2 = req;
        return send_(std::move(req2));
    }

    inline auto ClientImpl::send_(Request&& req) -> Result {
        auto res = detail::make_unique<Response>();
        auto error = Error::Success;
        const auto ret = send(req, *res, error);
        return Result{
            ret ? std::move(res) : nullptr,
            error,
            std::move(req.headers)
        };
    }

    inline auto ClientImpl::handle_request(Stream& strm, Request& req, Response& res, const bool close_connection, Error& error) -> bool {
        if (req.path.empty()) {
            error = Error::Connection;
            return false;
        }

        const auto req_save = req;

        bool ret;

        if (!is_ssl() && !proxy_host_.empty() && proxy_port_ != -1) {
            auto req2 = req;
            req2.path = "http://" + host_and_port_ + req.path;
            ret = process_request(strm, req2, res, close_connection, error);
            req = req2;
            req.path = req_save.path;
        } else {
            ret = process_request(strm, req, res, close_connection, error);
        }

        if (!ret) {
            return false;
        }

        if (res.get_header_value("Connection") == "close" || (res.version == "HTTP/1.0" && res.reason != "Connection established")) {
            // TODO this requires a not-entirely-obvious chain of calls to be correct
            // for this to be safe.

            // This is safe to call because handle_request is only called by send_
            // which locks the request mutex during the process. It would be a bug
            // to call it from a different thread since it's a thread-safety issue
            // to do these things to the socket if another thread is using the socket.
            std::lock_guard guard(socket_mutex_);
            shutdown_ssl(socket_, true);
            shutdown_socket(socket_);
            close_socket(socket_);
        }

        if (300 < res.status && res.status < 400 && follow_location_) {
            req = req_save;
            ret = redirect(req, res, error);
        }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        if ((res.status == Unauthorized_401 || res.status == ProxyAuthenticationRequired_407) && req.authorization_count_ < 5) {
            const auto is_proxy = res.status == ProxyAuthenticationRequired_407;
            const auto& username = is_proxy ? proxy_digest_auth_username_ : digest_auth_username_;
            const auto& password = is_proxy ? proxy_digest_auth_password_ : digest_auth_password_;

            if (!username.empty() && !password.empty()) {
                std::map<std::string, std::string> auth;
                if (detail::parse_www_authenticate(res, auth, is_proxy)) {
                    Request new_req = req;
                    new_req.authorization_count_ += 1;
                    new_req.headers.erase(is_proxy ? "Proxy-Authorization" : "Authorization");
                    new_req.headers.insert(detail::make_digest_authentication_header(
                        req,
                        auth,
                        new_req.authorization_count_,
                        detail::random_string(10),
                        username,
                        password,
                        is_proxy));

                    Response new_res;

                    ret = send(new_req, new_res, error);
                    if (ret)
                        res = new_res;
                }
            }
        }
#endif

        return ret;
    }

    inline auto ClientImpl::redirect(Request& req, Response& res, Error& error) -> bool {
        if (req.redirect_count_ == 0) {
            error = Error::ExceedRedirectCount;
            return false;
        }

        auto location = res.get_header_value("location");
        if (location.empty()) {
            return false;
        }

        static const std::regex re(R"((?:(https?):)?(?://(?:\[([a-fA-F\d:]+)\]|([^:/?#]+))(?::(\d+))?)?([^?#]*)(\?[^#]*)?(?:#.*)?)");

        std::smatch m;
        if (!std::regex_match(location, m, re)) {
            return false;
        }

        auto scheme = is_ssl() ? "https" : "http";

        auto next_scheme = m[1].str();
        auto next_host = m[2].str();
        if (next_host.empty()) {
            next_host = m[3].str();
        }
        auto port_str = m[4].str();
        auto next_path = m[5].str();
        auto next_query = m[6].str();

        auto next_port = port_;
        if (!port_str.empty()) {
            next_port = std::stoi(port_str);
        } else if (!next_scheme.empty()) {
            next_port = next_scheme == "https" ? 443 : 80;
        }

        if (next_scheme.empty()) {
            next_scheme = scheme;
        }
        if (next_host.empty()) {
            next_host = host_;
        }
        if (next_path.empty()) {
            next_path = "/";
        }

        auto path = detail::decode_url(next_path, true) + next_query;

        if (next_scheme == scheme && next_host == host_ && next_port == port_) {
            return detail::redirect(*this, req, res, path, location, error);
        }
        if (next_scheme == "https") {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
            SSLClient cli(next_host, next_port);
            cli.copy_settings(*this);
            if (ca_cert_store_)
                cli.set_ca_cert_store(ca_cert_store_);
            return detail::redirect(cli, req, res, path, location, error);
#else
      return false;
#endif
        }
        ClientImpl cli(next_host, next_port);
        cli.copy_settings(*this);
        return detail::redirect(cli, req, res, path, location, error);
    }

    inline auto ClientImpl::write_content_with_provider(Stream& strm, const Request& req, Error& error) const -> bool {
        auto is_shutting_down = [] { return false; };

        if (req.is_chunked_content_provider_) {
            // TODO: Brotli support
            std::unique_ptr<detail::compressor> compressor;
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
            if (compress_)
                compressor = detail::make_unique<detail::gzip_compressor>();
            else
#endif
                compressor = detail::make_unique<detail::nocompressor>();

            return write_content_chunked(strm, req.content_provider_, is_shutting_down, *compressor, error);
        }
        return detail::write_content(strm, req.content_provider_, 0, req.content_length_, is_shutting_down, error);
    }

    inline auto ClientImpl::write_request(Stream& strm, Request& req, const bool close_connection, Error& error) const -> bool {
        // Prepare additional headers
        if (close_connection) {
            if (!req.has_header("Connection")) {
                req.set_header("Connection", "close");
            }
        }

        if (!req.has_header("Host")) {
            if (is_ssl()) {
                if (port_ == 443) {
                    req.set_header("Host", host_);
                } else {
                    req.set_header("Host", host_and_port_);
                }
            } else {
                if (port_ == 80) {
                    req.set_header("Host", host_);
                } else {
                    req.set_header("Host", host_and_port_);
                }
            }
        }

        if (!req.has_header("Accept")) {
            req.set_header("Accept", "*/*");
        }

        if (!req.content_receiver) {
            if (!req.has_header("Accept-Encoding")) {
                std::string accept_encoding;
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
                accept_encoding = "br";
#endif
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
                if (!accept_encoding.empty())
                    accept_encoding += ", ";
                accept_encoding += "gzip, deflate";
#endif
                req.set_header("Accept-Encoding", accept_encoding);
            }

#ifndef CPPHTTPLIB_NO_DEFAULT_USER_AGENT
            if (!req.has_header("User-Agent")) {
                const auto agent = std::string("cpp-httplib/") + CPPHTTPLIB_VERSION;
                req.set_header("User-Agent", agent);
            }
#endif
        }

        if (req.body.empty()) {
            if (req.content_provider_) {
                if (!req.is_chunked_content_provider_) {
                    if (!req.has_header("Content-Length")) {
                        const auto length = std::to_string(req.content_length_);
                        req.set_header("Content-Length", length);
                    }
                }
            } else {
                if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH") {
                    req.set_header("Content-Length", "0");
                }
            }
        } else {
            if (!req.has_header("Content-Type")) {
                req.set_header("Content-Type", "text/plain");
            }

            if (!req.has_header("Content-Length")) {
                const auto length = std::to_string(req.body.size());
                req.set_header("Content-Length", length);
            }
        }

        if (!basic_auth_password_.empty() || !basic_auth_username_.empty()) {
            if (!req.has_header("Authorization")) {
                req.headers.insert(make_basic_authentication_header(basic_auth_username_, basic_auth_password_, false));
            }
        }

        if (!proxy_basic_auth_username_.empty() && !proxy_basic_auth_password_.empty()) {
            if (!req.has_header("Proxy-Authorization")) {
                req.headers.insert(make_basic_authentication_header(proxy_basic_auth_username_, proxy_basic_auth_password_, true));
            }
        }

        if (!bearer_token_auth_token_.empty()) {
            if (!req.has_header("Authorization")) {
                req.headers.insert(make_bearer_token_authentication_header(bearer_token_auth_token_, false));
            }
        }

        if (!proxy_bearer_token_auth_token_.empty()) {
            if (!req.has_header("Proxy-Authorization")) {
                req.headers.insert(make_bearer_token_authentication_header(proxy_bearer_token_auth_token_, true));
            }
        }

        // Request line and headers
        {
            detail::BufferStream bstrm;

            const auto& path_with_query = req.params.empty() ? req.path : append_query_params(req.path, req.params);

            const auto& path = url_encode_ ? detail::encode_url(path_with_query) : path_with_query;

            write_request_line(bstrm, req.method, path);

            header_writer_(bstrm, req.headers);

            // Flush buffer
            auto& data = bstrm.get_buffer();
            if (!detail::write_data(strm, data.data(), data.size())) {
                error = Error::Write;
                return false;
            }
        }

        // Body
        if (req.body.empty()) {
            return write_content_with_provider(strm, req, error);
        }

        if (!detail::write_data(strm, req.body.data(), req.body.size())) {
            error = Error::Write;
            return false;
        }

        return true;
    }

    inline auto ClientImpl::send_with_content_provider(Request& req,
                                                       const char* body,
                                                       const size_t content_length,
                                                       ContentProvider content_provider,
                                                       ContentProviderWithoutLength content_provider_without_length,
                                                       const std::string& content_type,
                                                       Error& error) -> std::unique_ptr<Response> {
        if (!content_type.empty()) {
            req.set_header("Content-Type", content_type);
        }

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
        if (compress_)
            req.set_header("Content-Encoding", "gzip");
#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
        if (compress_ && !content_provider_without_length) {
            // TODO: Brotli support
            detail::gzip_compressor compressor;

            if (content_provider) {
                auto ok = true;
                size_t offset = 0;
                DataSink data_sink;

                data_sink.write = [&](const char* data, const size_t data_len) -> bool {
                    if (ok) {
                        const auto last = offset + data_len == content_length;

                        const auto ret = compressor.compress(data,
                                                             data_len,
                                                             last,
                                                             [&](const char* compressed_data, const size_t compressed_data_len) {
                                                                 req.body.append(compressed_data, compressed_data_len);
                                                                 return true;
                                                             });

                        if (ret)
                            offset += data_len;
                        else
                            ok = false;
                    }
                    return ok;
                };

                while (ok && offset < content_length) {
                    if (!content_provider(offset, content_length - offset, data_sink)) {
                        error = Error::Canceled;
                        return nullptr;
                    }
                }
            } else {
                if (!compressor.compress(body,
                                         content_length,
                                         true,
                                         [&](const char* data, const size_t data_len) {
                                             req.body.append(data, data_len);
                                             return true;
                                         })) {
                    error = Error::Compression;
                    return nullptr;
                }
            }
        } else
#endif
        {
            if (content_provider) {
                req.content_length_ = content_length;
                req.content_provider_ = std::move(content_provider);
                req.is_chunked_content_provider_ = false;
            } else if (content_provider_without_length) {
                req.content_length_ = 0;
                req.content_provider_ = detail::ContentProviderAdapter(std::move(content_provider_without_length));
                req.is_chunked_content_provider_ = true;
                req.set_header("Transfer-Encoding", "chunked");
            } else {
                req.body.assign(body, content_length);
            }
        }

        auto res = detail::make_unique<Response>();
        return send(req, *res, error) ? std::move(res) : nullptr;
    }

    inline auto ClientImpl::send_with_content_provider(const std::string& method,
                                                       const std::string& path,
                                                       const Headers& headers,
                                                       const char* body,
                                                       const size_t content_length,
                                                       ContentProvider content_provider,
                                                       ContentProviderWithoutLength content_provider_without_length,
                                                       const std::string& content_type,
                                                       const Progress& progress) -> Result {
        Request req;
        req.method = method;
        req.headers = headers;
        req.path = path;
        req.progress = progress;
        if (max_timeout_msec_ > 0) {
            req.start_time_ = std::chrono::steady_clock::now();
        }

        auto error = Error::Success;

        auto res = send_with_content_provider(req,
                                              body,
                                              content_length,
                                              std::move(content_provider),
                                              std::move(content_provider_without_length),
                                              content_type,
                                              error);

        return Result{
            std::move(res),
            error,
            std::move(req.headers)
        };
    }

    inline auto ClientImpl::adjust_host_string(const std::string& host) -> std::string {
        if (host.find(':') != std::string::npos) {
            return "[" + host + "]";
        }
        return host;
    }

    inline auto ClientImpl::process_request(Stream& strm, Request& req, Response& res, const bool close_connection, Error& error) const -> bool {
        // Send request
        if (!write_request(strm, req, close_connection, error)) {
            return false;
        }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        if (is_ssl()) {
            const auto is_proxy_enabled = !proxy_host_.empty() && proxy_port_ != -1;
            if (!is_proxy_enabled) {
                if (detail::is_ssl_peer_could_be_closed(socket_.ssl, socket_.sock)) {
                    error = Error::SSLPeerCouldBeClosed_;
                    return false;
                }
            }
        }
#endif

        // Receive response and headers
        if (!read_response_line(strm, req, res) || !detail::read_headers(strm, res.headers)) {
            error = Error::Read;
            return false;
        }

        // Body
        if (res.status != NoContent_204 && req.method != "HEAD" && req.method != "CONNECT") {
            const auto redirect = 300 < res.status && res.status < 400 && res.status != NotModified_304 && follow_location_;

            if (req.response_handler && !redirect) {
                if (!req.response_handler(res)) {
                    error = Error::Canceled;
                    return false;
                }
            }

            const auto out = req.content_receiver
                           ? static_cast<ContentReceiverWithProgress>([&](const char* buf, const size_t n, const uint64_t off, const uint64_t len) {
                               if (redirect) {
                                   return true;
                               }
                               const auto ret = req.content_receiver(buf, n, off, len);
                               if (!ret) {
                                   error = Error::Canceled;
                               }
                               return ret;
                           })
                           : static_cast<ContentReceiverWithProgress>([&](const char* buf, const size_t n, uint64_t /*off*/, uint64_t /*len*/) {
                               assert(res.body.size() + n <= res.body.max_size());
                               res.body.append(buf, n);
                               return true;
                           });

            auto progress = [&](const uint64_t current, const uint64_t total) {
                if (!req.progress || redirect) {
                    return true;
                }
                const auto ret = req.progress(current, total);
                if (!ret) {
                    error = Error::Canceled;
                }
                return ret;
            };

            if (res.has_header("Content-Length")) {
                if (!req.content_receiver) {
                    const auto len = res.get_header_value_u64("Content-Length");
                    if (len > res.body.max_size()) {
                        error = Error::Read;
                        return false;
                    }
                    res.body.reserve(len);
                }
            }

            if (res.status != NotModified_304) {
                int dummy_status;
                if (!detail::read_content(strm,
                                          res,
                                          (std::numeric_limits<size_t>::max)(),
                                          dummy_status,
                                          std::move(progress),
                                          out,
                                          decompress_)) {
                    if (error != Error::Canceled) {
                        error = Error::Read;
                    }
                    return false;
                }
            }
        }

        // Log
        if (logger_) {
            logger_(req, res);
        }

        return true;
    }

    inline auto ClientImpl::get_multipart_content_provider(const std::string& boundary,
                                                           const MultipartFormDataItems& items,
                                                           const MultipartFormDataProviderItems& provider_items) -> ContentProviderWithoutLength {
        size_t cur_item = 0;
        size_t cur_start = 0;
        // cur_item and cur_start are copied to within the std::function and maintain
        // state between successive calls
        return [&, cur_item, cur_start](size_t offset, DataSink& sink) mutable -> bool {
            if (!offset && !items.empty()) {
                sink.os << detail::serialize_multipart_formdata(items, boundary, false);
                return true;
            }
            if (cur_item < provider_items.size()) {
                if (!cur_start) {
                    const auto& begin = detail::serialize_multipart_formdata_item_begin(provider_items[cur_item], boundary);
                    offset += begin.size();
                    cur_start = offset;
                    sink.os << begin;
                }

                DataSink cur_sink;
                auto has_data = true;
                cur_sink.write = sink.write;
                cur_sink.done = [&] { has_data = false; };

                if (!provider_items[cur_item].provider(offset - cur_start, cur_sink)) {
                    return false;
                }

                if (!has_data) {
                    sink.os << detail::serialize_multipart_formdata_item_end();
                    cur_item++;
                    cur_start = 0;
                }
                return true;
            }
            sink.os << detail::serialize_multipart_formdata_finish(boundary);
            sink.done();
            return true;
        };
    }

    inline auto ClientImpl::process_socket(const Socket& socket,
                                           const std::chrono::time_point<std::chrono::steady_clock> start_time,
                                           const std::function<bool(Stream& strm)> callback) -> bool {
        return detail::process_client_socket(socket.sock,
                                             read_timeout_sec_,
                                             read_timeout_usec_,
                                             write_timeout_sec_,
                                             write_timeout_usec_,
                                             max_timeout_msec_,
                                             start_time,
                                             callback);
    }

    inline auto ClientImpl::is_ssl() const -> bool { return false; }

    inline auto ClientImpl::Get(const std::string& path) -> Result { return Get(path, Headers(), Progress()); }

    inline auto ClientImpl::Get(const std::string& path, Progress progress) -> Result { return Get(path, Headers(), std::move(progress)); }

    inline auto ClientImpl::Get(const std::string& path, const Headers& headers) -> Result { return Get(path, headers, Progress()); }

    inline auto ClientImpl::Get(const std::string& path, const Headers& headers, Progress progress) -> Result {
        Request req;
        req.method = "GET";
        req.path = path;
        req.headers = headers;
        req.progress = std::move(progress);
        if (max_timeout_msec_ > 0) {
            req.start_time_ = std::chrono::steady_clock::now();
        }

        return send_(std::move(req));
    }

    inline auto ClientImpl::Get(const std::string& path, const ContentReceiver& content_receiver) -> Result {
        return Get(path, Headers(), nullptr, content_receiver, nullptr);
    }

    inline auto ClientImpl::Get(const std::string& path, const ContentReceiver& content_receiver, Progress progress) -> Result {
        return Get(path, Headers(), nullptr, content_receiver, std::move(progress));
    }

    inline auto ClientImpl::Get(const std::string& path, const Headers& headers, const ContentReceiver& content_receiver) -> Result {
        return Get(path, headers, nullptr, content_receiver, nullptr);
    }

    inline auto ClientImpl::Get(const std::string& path,
                                const Headers& headers,
                                const ContentReceiver& content_receiver,
                                Progress progress) -> Result { return Get(path, headers, nullptr, content_receiver, std::move(progress)); }

    inline auto ClientImpl::Get(const std::string& path, ResponseHandler response_handler, const ContentReceiver& content_receiver) -> Result {
        return Get(path, Headers(), std::move(response_handler), content_receiver, nullptr);
    }

    inline auto ClientImpl::Get(const std::string& path,
                                const Headers& headers,
                                ResponseHandler response_handler,
                                const ContentReceiver& content_receiver) -> Result {
        return Get(path, headers, std::move(response_handler), content_receiver, nullptr);
    }

    inline auto ClientImpl::Get(const std::string& path,
                                ResponseHandler response_handler,
                                const ContentReceiver& content_receiver,
                                Progress progress) -> Result {
        return Get(path, Headers(), std::move(response_handler), content_receiver, std::move(progress));
    }

    inline auto ClientImpl::Get(const std::string& path,
                                const Headers& headers,
                                ResponseHandler response_handler,
                                const ContentReceiver& content_receiver,
                                Progress progress) -> Result {
        Request req;
        req.method = "GET";
        req.path = path;
        req.headers = headers;
        req.response_handler = std::move(response_handler);
        req.content_receiver = [content_receiver](const char* data, const size_t data_length, uint64_t /*offset*/, uint64_t /*total_length*/) {
            return content_receiver(data, data_length);
        };
        req.progress = std::move(progress);
        if (max_timeout_msec_ > 0) {
            req.start_time_ = std::chrono::steady_clock::now();
        }

        return send_(std::move(req));
    }

    inline auto ClientImpl::Get(const std::string& path, const Params& params, const Headers& headers, Progress progress) -> Result {
        if (params.empty()) {
            return Get(path, headers);
        }

        const std::string path_with_query = append_query_params(path, params);
        return Get(path_with_query, headers, std::move(progress));
    }

    inline auto ClientImpl::Get(const std::string& path,
                                const Params& params,
                                const Headers& headers,
                                const ContentReceiver& content_receiver,
                                Progress progress) -> Result {
        return Get(path, params, headers, nullptr, content_receiver, std::move(progress));
    }

    inline auto ClientImpl::Get(const std::string& path,
                                const Params& params,
                                const Headers& headers,
                                ResponseHandler response_handler,
                                const ContentReceiver& content_receiver,
                                Progress progress) -> Result {
        if (params.empty()) {
            return Get(path, headers, std::move(response_handler), content_receiver, std::move(progress));
        }

        const std::string path_with_query = append_query_params(path, params);
        return Get(path_with_query, headers, std::move(response_handler), content_receiver, std::move(progress));
    }

    inline auto ClientImpl::Head(const std::string& path) -> Result { return Head(path, Headers()); }

    inline auto ClientImpl::Head(const std::string& path, const Headers& headers) -> Result {
        Request req;
        req.method = "HEAD";
        req.headers = headers;
        req.path = path;
        if (max_timeout_msec_ > 0) {
            req.start_time_ = std::chrono::steady_clock::now();
        }

        return send_(std::move(req));
    }

    inline auto ClientImpl::Post(const std::string& path) -> Result { return Post(path, std::string(), std::string()); }

    inline auto ClientImpl::Post(const std::string& path, const Headers& headers) -> Result { return Post(path, headers, nullptr, 0, std::string()); }

    inline auto ClientImpl::Post(const std::string& path, const char* body, const size_t content_length, const std::string& content_type) -> Result {
        return Post(path, Headers(), body, content_length, content_type, nullptr);
    }

    inline auto ClientImpl::Post(const std::string& path,
                                 const Headers& headers,
                                 const char* body,
                                 const size_t content_length,
                                 const std::string& content_type) -> Result {
        return send_with_content_provider("POST", path, headers, body, content_length, nullptr, nullptr, content_type, nullptr);
    }

    inline auto ClientImpl::Post(const std::string& path,
                                 const Headers& headers,
                                 const char* body,
                                 const size_t content_length,
                                 const std::string& content_type,
                                 const Progress& progress) -> Result {
        return send_with_content_provider("POST", path, headers, body, content_length, nullptr, nullptr, content_type, progress);
    }

    inline auto ClientImpl::Post(const std::string& path, const std::string& body, const std::string& content_type) -> Result {
        return Post(path, Headers(), body, content_type);
    }

    inline auto ClientImpl::Post(const std::string& path,
                                 const std::string& body,
                                 const std::string& content_type,
                                 const Progress& progress) -> Result { return Post(path, Headers(), body, content_type, progress); }

    inline auto
    ClientImpl::Post(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) -> Result {
        return send_with_content_provider("POST", path, headers, body.data(), body.size(), nullptr, nullptr, content_type, nullptr);
    }

    inline auto ClientImpl::Post(const std::string& path,
                                 const Headers& headers,
                                 const std::string& body,
                                 const std::string& content_type,
                                 const Progress& progress) -> Result {
        return send_with_content_provider("POST", path, headers, body.data(), body.size(), nullptr, nullptr, content_type, progress);
    }

    inline auto ClientImpl::Post(const std::string& path, const Params& params) -> Result { return Post(path, Headers(), params); }

    inline auto ClientImpl::Post(const std::string& path,
                                 const size_t content_length,
                                 ContentProvider content_provider,
                                 const std::string& content_type) -> Result {
        return Post(path, Headers(), content_length, std::move(content_provider), content_type);
    }

    inline auto ClientImpl::Post(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) -> Result {
        return Post(path, Headers(), std::move(content_provider), content_type);
    }

    inline auto ClientImpl::Post(const std::string& path,
                                 const Headers& headers,
                                 const size_t content_length,
                                 ContentProvider content_provider,
                                 const std::string& content_type) -> Result {
        return send_with_content_provider("POST",
                                          path,
                                          headers,
                                          nullptr,
                                          content_length,
                                          std::move(content_provider),
                                          nullptr,
                                          content_type,
                                          nullptr);
    }

    inline auto ClientImpl::Post(const std::string& path,
                                 const Headers& headers,
                                 ContentProviderWithoutLength content_provider,
                                 const std::string& content_type) -> Result {
        return send_with_content_provider("POST", path, headers, nullptr, 0, nullptr, std::move(content_provider), content_type, nullptr);
    }

    inline auto ClientImpl::Post(const std::string& path, const Headers& headers, const Params& params) -> Result {
        const auto query = detail::params_to_query_str(params);
        return Post(path, headers, query, "application/x-www-form-urlencoded");
    }

    inline auto ClientImpl::Post(const std::string& path, const Headers& headers, const Params& params, const Progress& progress) -> Result {
        const auto query = detail::params_to_query_str(params);
        return Post(path, headers, query, "application/x-www-form-urlencoded", progress);
    }

    inline auto ClientImpl::Post(const std::string& path, const MultipartFormDataItems& items) -> Result { return Post(path, Headers(), items); }

    inline auto ClientImpl::Post(const std::string& path, const Headers& headers, const MultipartFormDataItems& items) -> Result {
        const auto& boundary = detail::make_multipart_data_boundary();
        const auto& content_type = detail::serialize_multipart_formdata_get_content_type(boundary);
        const auto& body = detail::serialize_multipart_formdata(items, boundary);
        return Post(path, headers, body, content_type);
    }

    inline auto ClientImpl::Post(const std::string& path,
                                 const Headers& headers,
                                 const MultipartFormDataItems& items,
                                 const std::string& boundary) -> Result {
        if (!detail::is_multipart_boundary_chars_valid(boundary)) {
            return Result{
                nullptr,
                Error::UnsupportedMultipartBoundaryChars
            };
        }

        const auto& content_type = detail::serialize_multipart_formdata_get_content_type(boundary);
        const auto& body = detail::serialize_multipart_formdata(items, boundary);
        return Post(path, headers, body, content_type);
    }

    inline auto ClientImpl::Post(const std::string& path,
                                 const Headers& headers,
                                 const MultipartFormDataItems& items,
                                 const MultipartFormDataProviderItems& provider_items) -> Result {
        const auto& boundary = detail::make_multipart_data_boundary();
        const auto& content_type = detail::serialize_multipart_formdata_get_content_type(boundary);
        return send_with_content_provider("POST",
                                          path,
                                          headers,
                                          nullptr,
                                          0,
                                          nullptr,
                                          get_multipart_content_provider(boundary, items, provider_items),
                                          content_type,
                                          nullptr);
    }

    inline auto ClientImpl::Put(const std::string& path) -> Result { return Put(path, std::string(), std::string()); }

    inline auto ClientImpl::Put(const std::string& path, const char* body, const size_t content_length, const std::string& content_type) -> Result {
        return Put(path, Headers(), body, content_length, content_type);
    }

    inline auto ClientImpl::Put(const std::string& path,
                                const Headers& headers,
                                const char* body,
                                const size_t content_length,
                                const std::string& content_type) -> Result {
        return send_with_content_provider("PUT", path, headers, body, content_length, nullptr, nullptr, content_type, nullptr);
    }

    inline auto ClientImpl::Put(const std::string& path,
                                const Headers& headers,
                                const char* body,
                                const size_t content_length,
                                const std::string& content_type,
                                const Progress& progress) -> Result {
        return send_with_content_provider("PUT", path, headers, body, content_length, nullptr, nullptr, content_type, progress);
    }

    inline auto ClientImpl::Put(const std::string& path, const std::string& body, const std::string& content_type) -> Result {
        return Put(path, Headers(), body, content_type);
    }

    inline auto ClientImpl::Put(const std::string& path,
                                const std::string& body,
                                const std::string& content_type,
                                const Progress& progress) -> Result { return Put(path, Headers(), body, content_type, progress); }

    inline auto ClientImpl::Put(const std::string& path, const Headers& headers, const std::string& body, const std::string& content_type) -> Result {
        return send_with_content_provider("PUT", path, headers, body.data(), body.size(), nullptr, nullptr, content_type, nullptr);
    }

    inline auto ClientImpl::Put(const std::string& path,
                                const Headers& headers,
                                const std::string& body,
                                const std::string& content_type,
                                const Progress& progress) -> Result {
        return send_with_content_provider("PUT", path, headers, body.data(), body.size(), nullptr, nullptr, content_type, progress);
    }

    inline auto ClientImpl::Put(const std::string& path,
                                const size_t content_length,
                                ContentProvider content_provider,
                                const std::string& content_type) -> Result {
        return Put(path, Headers(), content_length, std::move(content_provider), content_type);
    }

    inline auto ClientImpl::Put(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) -> Result {
        return Put(path, Headers(), std::move(content_provider), content_type);
    }

    inline auto ClientImpl::Put(const std::string& path,
                                const Headers& headers,
                                const size_t content_length,
                                ContentProvider content_provider,
                                const std::string& content_type) -> Result {
        return send_with_content_provider("PUT", path, headers, nullptr, content_length, std::move(content_provider), nullptr, content_type, nullptr);
    }

    inline auto ClientImpl::Put(const std::string& path,
                                const Headers& headers,
                                ContentProviderWithoutLength content_provider,
                                const std::string& content_type) -> Result {
        return send_with_content_provider("PUT", path, headers, nullptr, 0, nullptr, std::move(content_provider), content_type, nullptr);
    }

    inline auto ClientImpl::Put(const std::string& path, const Params& params) -> Result { return Put(path, Headers(), params); }

    inline auto ClientImpl::Put(const std::string& path, const Headers& headers, const Params& params) -> Result {
        const auto query = detail::params_to_query_str(params);
        return Put(path, headers, query, "application/x-www-form-urlencoded");
    }

    inline auto ClientImpl::Put(const std::string& path, const Headers& headers, const Params& params, const Progress& progress) -> Result {
        const auto query = detail::params_to_query_str(params);
        return Put(path, headers, query, "application/x-www-form-urlencoded", progress);
    }

    inline auto ClientImpl::Put(const std::string& path, const MultipartFormDataItems& items) -> Result { return Put(path, Headers(), items); }

    inline auto ClientImpl::Put(const std::string& path, const Headers& headers, const MultipartFormDataItems& items) -> Result {
        const auto& boundary = detail::make_multipart_data_boundary();
        const auto& content_type = detail::serialize_multipart_formdata_get_content_type(boundary);
        const auto& body = detail::serialize_multipart_formdata(items, boundary);
        return Put(path, headers, body, content_type);
    }

    inline auto ClientImpl::Put(const std::string& path,
                                const Headers& headers,
                                const MultipartFormDataItems& items,
                                const std::string& boundary) -> Result {
        if (!detail::is_multipart_boundary_chars_valid(boundary)) {
            return Result{
                nullptr,
                Error::UnsupportedMultipartBoundaryChars
            };
        }

        const auto& content_type = detail::serialize_multipart_formdata_get_content_type(boundary);
        const auto& body = detail::serialize_multipart_formdata(items, boundary);
        return Put(path, headers, body, content_type);
    }

    inline auto ClientImpl::Put(const std::string& path,
                                const Headers& headers,
                                const MultipartFormDataItems& items,
                                const MultipartFormDataProviderItems& provider_items) -> Result {
        const auto& boundary = detail::make_multipart_data_boundary();
        const auto& content_type = detail::serialize_multipart_formdata_get_content_type(boundary);
        return send_with_content_provider("PUT",
                                          path,
                                          headers,
                                          nullptr,
                                          0,
                                          nullptr,
                                          get_multipart_content_provider(boundary, items, provider_items),
                                          content_type,
                                          nullptr);
    }

    inline auto ClientImpl::Patch(const std::string& path) -> Result { return Patch(path, std::string(), std::string()); }

    inline auto ClientImpl::Patch(const std::string& path, const char* body, const size_t content_length, const std::string& content_type) -> Result {
        return Patch(path, Headers(), body, content_length, content_type);
    }

    inline auto ClientImpl::Patch(const std::string& path,
                                  const char* body,
                                  const size_t content_length,
                                  const std::string& content_type,
                                  const Progress& progress) -> Result { return Patch(path, Headers(), body, content_length, content_type, progress); }

    inline auto ClientImpl::Patch(const std::string& path,
                                  const Headers& headers,
                                  const char* body,
                                  const size_t content_length,
                                  const std::string& content_type) -> Result {
        return Patch(path, headers, body, content_length, content_type, nullptr);
    }

    inline auto ClientImpl::Patch(const std::string& path,
                                  const Headers& headers,
                                  const char* body,
                                  const size_t content_length,
                                  const std::string& content_type,
                                  const Progress& progress) -> Result {
        return send_with_content_provider("PATCH", path, headers, body, content_length, nullptr, nullptr, content_type, progress);
    }

    inline auto ClientImpl::Patch(const std::string& path, const std::string& body, const std::string& content_type) -> Result {
        return Patch(path, Headers(), body, content_type);
    }

    inline auto ClientImpl::Patch(const std::string& path,
                                  const std::string& body,
                                  const std::string& content_type,
                                  const Progress& progress) -> Result { return Patch(path, Headers(), body, content_type, progress); }

    inline auto ClientImpl::Patch(const std::string& path,
                                  const Headers& headers,
                                  const std::string& body,
                                  const std::string& content_type) -> Result { return Patch(path, headers, body, content_type, nullptr); }

    inline auto ClientImpl::Patch(const std::string& path,
                                  const Headers& headers,
                                  const std::string& body,
                                  const std::string& content_type,
                                  const Progress& progress) -> Result {
        return send_with_content_provider("PATCH", path, headers, body.data(), body.size(), nullptr, nullptr, content_type, progress);
    }

    inline auto ClientImpl::Patch(const std::string& path,
                                  const size_t content_length,
                                  ContentProvider content_provider,
                                  const std::string& content_type) -> Result {
        return Patch(path, Headers(), content_length, std::move(content_provider), content_type);
    }

    inline auto ClientImpl::Patch(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) -> Result {
        return Patch(path, Headers(), std::move(content_provider), content_type);
    }

    inline auto ClientImpl::Patch(const std::string& path,
                                  const Headers& headers,
                                  const size_t content_length,
                                  ContentProvider content_provider,
                                  const std::string& content_type) -> Result {
        return send_with_content_provider("PATCH",
                                          path,
                                          headers,
                                          nullptr,
                                          content_length,
                                          std::move(content_provider),
                                          nullptr,
                                          content_type,
                                          nullptr);
    }

    inline auto ClientImpl::Patch(const std::string& path,
                                  const Headers& headers,
                                  ContentProviderWithoutLength content_provider,
                                  const std::string& content_type) -> Result {
        return send_with_content_provider("PATCH", path, headers, nullptr, 0, nullptr, std::move(content_provider), content_type, nullptr);
    }

    inline auto ClientImpl::Delete(const std::string& path) -> Result { return Delete(path, Headers(), std::string(), std::string()); }

    inline auto ClientImpl::Delete(const std::string& path, const Headers& headers) -> Result {
        return Delete(path, headers, std::string(), std::string());
    }

    inline auto ClientImpl::Delete(const std::string& path,
                                   const char* body,
                                   const size_t content_length,
                                   const std::string& content_type) -> Result { return Delete(path, Headers(), body, content_length, content_type); }

    inline auto ClientImpl::Delete(const std::string& path,
                                   const char* body,
                                   const size_t content_length,
                                   const std::string& content_type,
                                   const Progress& progress) -> Result {
        return Delete(path, Headers(), body, content_length, content_type, progress);
    }

    inline auto ClientImpl::Delete(const std::string& path,
                                   const Headers& headers,
                                   const char* body,
                                   const size_t content_length,
                                   const std::string& content_type) -> Result {
        return Delete(path, headers, body, content_length, content_type, nullptr);
    }

    inline auto ClientImpl::Delete(const std::string& path,
                                   const Headers& headers,
                                   const char* body,
                                   const size_t content_length,
                                   const std::string& content_type,
                                   const Progress& progress) -> Result {
        Request req;
        req.method = "DELETE";
        req.headers = headers;
        req.path = path;
        req.progress = progress;
        if (max_timeout_msec_ > 0) {
            req.start_time_ = std::chrono::steady_clock::now();
        }

        if (!content_type.empty()) {
            req.set_header("Content-Type", content_type);
        }
        req.body.assign(body, content_length);

        return send_(std::move(req));
    }

    inline auto ClientImpl::Delete(const std::string& path, const std::string& body, const std::string& content_type) -> Result {
        return Delete(path, Headers(), body.data(), body.size(), content_type);
    }

    inline auto ClientImpl::Delete(const std::string& path,
                                   const std::string& body,
                                   const std::string& content_type,
                                   const Progress& progress) -> Result {
        return Delete(path, Headers(), body.data(), body.size(), content_type, progress);
    }

    inline auto ClientImpl::Delete(const std::string& path,
                                   const Headers& headers,
                                   const std::string& body,
                                   const std::string& content_type) -> Result {
        return Delete(path, headers, body.data(), body.size(), content_type);
    }

    inline auto ClientImpl::Delete(const std::string& path,
                                   const Headers& headers,
                                   const std::string& body,
                                   const std::string& content_type,
                                   const Progress& progress) -> Result {
        return Delete(path, headers, body.data(), body.size(), content_type, progress);
    }

    inline auto ClientImpl::Options(const std::string& path) -> Result { return Options(path, Headers()); }

    inline auto ClientImpl::Options(const std::string& path, const Headers& headers) -> Result {
        Request req;
        req.method = "OPTIONS";
        req.headers = headers;
        req.path = path;
        if (max_timeout_msec_ > 0) {
            req.start_time_ = std::chrono::steady_clock::now();
        }

        return send_(std::move(req));
    }

    inline auto ClientImpl::stop() -> void {
        std::lock_guard guard(socket_mutex_);

        // If there is anything ongoing right now, the ONLY thread-safe thing we can
        // do is to shutdown_socket, so that threads using this socket suddenly
        // discover they can't read/write any more and error out. Everything else
        // (closing the socket, shutting ssl down) is unsafe because these actions are
        // not thread-safe.
        if (socket_requests_in_flight_ > 0) {
            shutdown_socket(socket_);

            // Aside from that, we set a flag for the socket to be closed when we're
            // done.
            socket_should_be_closed_when_request_is_done_ = true;
            return;
        }

        // Otherwise, still holding the mutex, we can shut everything down ourselves
        shutdown_ssl(socket_, true);
        shutdown_socket(socket_);
        close_socket(socket_);
    }

    inline auto ClientImpl::host() const -> std::string { return host_; }

    inline auto ClientImpl::port() const -> int { return port_; }

    inline auto ClientImpl::is_socket_open() const -> size_t {
        std::lock_guard guard(socket_mutex_);
        return socket_.is_open();
    }

    inline auto ClientImpl::socket() const -> socket_t { return socket_.sock; }

    inline auto ClientImpl::set_connection_timeout(const time_t sec, const time_t usec) -> void {
        connection_timeout_sec_ = sec;
        connection_timeout_usec_ = usec;
    }

    inline auto ClientImpl::set_read_timeout(const time_t sec, const time_t usec) -> void {
        read_timeout_sec_ = sec;
        read_timeout_usec_ = usec;
    }

    inline auto ClientImpl::set_write_timeout(const time_t sec, const time_t usec) -> void {
        write_timeout_sec_ = sec;
        write_timeout_usec_ = usec;
    }

    inline auto ClientImpl::set_max_timeout(const time_t msec) -> void { max_timeout_msec_ = msec; }

    inline auto ClientImpl::set_basic_auth(const std::string& username, const std::string& password) -> void {
        basic_auth_username_ = username;
        basic_auth_password_ = password;
    }

    inline auto ClientImpl::set_bearer_token_auth(const std::string& token) -> void { bearer_token_auth_token_ = token; }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    inline auto ClientImpl::set_digest_auth(const std::string& username, const std::string& password) -> void {
        digest_auth_username_ = username;
        digest_auth_password_ = password;
    }
#endif

    inline auto ClientImpl::set_keep_alive(const bool on) -> void { keep_alive_ = on; }

    inline auto ClientImpl::set_follow_location(const bool on) -> void { follow_location_ = on; }

    inline auto ClientImpl::set_url_encode(const bool on) -> void { url_encode_ = on; }

    inline auto ClientImpl::set_hostname_addr_map(std::map<std::string, std::string> addr_map) -> void { addr_map_ = std::move(addr_map); }

    inline auto ClientImpl::set_default_headers(Headers headers) -> void { default_headers_ = std::move(headers); }

    inline auto ClientImpl::set_header_writer(const std::function<ssize_t(Stream&, Headers&)>& writer) -> void { header_writer_ = writer; }

    inline auto ClientImpl::set_address_family(const int family) -> void { address_family_ = family; }

    inline auto ClientImpl::set_tcp_nodelay(const bool on) -> void { tcp_nodelay_ = on; }

    inline auto ClientImpl::set_ipv6_v6only(const bool on) -> void { ipv6_v6only_ = on; }

    inline auto ClientImpl::set_socket_options(SocketOptions socket_options) -> void { socket_options_ = std::move(socket_options); }

    inline auto ClientImpl::set_compress(const bool on) -> void { compress_ = on; }

    inline auto ClientImpl::set_decompress(const bool on) -> void { decompress_ = on; }

    inline auto ClientImpl::set_interface(const std::string& intf) -> void { interface_ = intf; }

    inline auto ClientImpl::set_proxy(const std::string& host, const int port) -> void {
        proxy_host_ = host;
        proxy_port_ = port;
    }

    inline auto ClientImpl::set_proxy_basic_auth(const std::string& username, const std::string& password) -> void {
        proxy_basic_auth_username_ = username;
        proxy_basic_auth_password_ = password;
    }

    inline auto ClientImpl::set_proxy_bearer_token_auth(const std::string& token) -> void { proxy_bearer_token_auth_token_ = token; }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    inline auto ClientImpl::set_proxy_digest_auth(const std::string& username, const std::string& password) -> void {
        proxy_digest_auth_username_ = username;
        proxy_digest_auth_password_ = password;
    }

    inline auto ClientImpl::set_ca_cert_path(const std::string& ca_cert_file_path, const std::string& ca_cert_dir_path) -> void {
        ca_cert_file_path_ = ca_cert_file_path;
        ca_cert_dir_path_ = ca_cert_dir_path;
    }

    inline auto ClientImpl::set_ca_cert_store(X509_STORE* ca_cert_store) -> void {
        if (ca_cert_store && ca_cert_store != ca_cert_store_)
            ca_cert_store_ = ca_cert_store;
    }

    inline auto ClientImpl::create_ca_cert_store(const char* ca_cert, const std::size_t size) -> X509_STORE* {
        const auto mem = BIO_new_mem_buf(ca_cert, static_cast<int>(size));
        auto se = detail::scope_exit([&] { BIO_free_all(mem); });
        if (!mem)
            return nullptr;

        const auto inf = PEM_X509_INFO_read_bio(mem, nullptr, nullptr, nullptr);
        if (!inf)
            return nullptr;

        const auto cts = X509_STORE_new();
        if (cts) {
            for (auto i = 0; i < sk_X509_INFO_num(inf); i++) {
                const auto itmp = sk_X509_INFO_value(inf, i);
                if (!itmp)
                    continue;

                if (itmp->x509)
                    X509_STORE_add_cert(cts, itmp->x509);
                if (itmp->crl)
                    X509_STORE_add_crl(cts, itmp->crl);
            }
        }

        sk_X509_INFO_pop_free(inf, X509_INFO_free);
        return cts;
    }

    inline auto ClientImpl::enable_server_certificate_verification(const bool enabled) -> void { server_certificate_verification_ = enabled; }

    inline auto ClientImpl::enable_server_hostname_verification(const bool enabled) -> void { server_hostname_verification_ = enabled; }

    inline auto ClientImpl::set_server_certificate_verifier(std::function<SSLVerifierResponse(SSL* ssl)> verifier) -> void {
        server_certificate_verifier_ = std::move(verifier);
    }
#endif

    inline auto ClientImpl::set_logger(Logger logger) -> void { logger_ = std::move(logger); }

    /*
     * SSL Implementation
     */
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    namespace detail {
        template <typename U, typename V>
        auto ssl_new(const socket_t sock, SSL_CTX* ctx, std::mutex& ctx_mutex, U SSL_connect_or_accept, V setup) -> SSL* {
            SSL* ssl = nullptr;
            {
                std::lock_guard guard(ctx_mutex);
                ssl = SSL_new(ctx);
            }

            if (ssl) {
                set_nonblocking(sock, true);
                const auto bio = BIO_new_socket(static_cast<int>(sock), BIO_NOCLOSE);
                BIO_set_nbio(bio, 1);
                SSL_set_bio(ssl, bio, bio);

                if (!setup(ssl) || SSL_connect_or_accept(ssl) != 1) {
                    SSL_shutdown(ssl);
                    {
                        std::lock_guard guard(ctx_mutex);
                        SSL_free(ssl);
                    }
                    set_nonblocking(sock, false);
                    return nullptr;
                }
                BIO_set_nbio(bio, 0);
                set_nonblocking(sock, false);
            }

            return ssl;
        }

        inline auto ssl_delete(std::mutex& ctx_mutex, SSL* ssl, socket_t sock, const bool shutdown_gracefully) -> void {
            // sometimes we may want to skip this to try to avoid SIGPIPE if we know
            // the remote has closed the network connection
            // Note that it is not always possible to avoid SIGPIPE, this is merely a
            // best-efforts.
            if (shutdown_gracefully) {
#ifdef _WIN32
                (void)sock;
                SSL_shutdown(ssl);
#else
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<const void *>(&tv), sizeof(tv));

    auto ret = SSL_shutdown(ssl);
    while (ret == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds{100});
      ret = SSL_shutdown(ssl);
    }
#endif
            }

            std::lock_guard guard(ctx_mutex);
            SSL_free(ssl);
        }

        template <typename U>
        auto ssl_connect_or_accept_nonblocking(const socket_t sock,
                                               SSL* ssl,
                                               U ssl_connect_or_accept,
                                               const time_t timeout_sec,
                                               const time_t timeout_usec) -> bool {
            auto res = 0;
            while ((res = ssl_connect_or_accept(ssl)) != 1) {
                const auto err = SSL_get_error(ssl, res);
                switch (err) {
                    case SSL_ERROR_WANT_READ:
                        if (select_read(sock, timeout_sec, timeout_usec) > 0)
                            continue;
                        break;
                    case SSL_ERROR_WANT_WRITE:
                        if (select_write(sock, timeout_sec, timeout_usec) > 0)
                            continue;
                        break;
                    default:
                        break;
                }
                return false;
            }
            return true;
        }

        template <typename T>
        auto process_server_socket_ssl(const std::atomic<socket_t>& svr_sock,
                                       SSL* ssl,
                                       socket_t sock,
                                       size_t keep_alive_max_count,
                                       time_t keep_alive_timeout_sec,
                                       const time_t read_timeout_sec,
                                       const time_t read_timeout_usec,
                                       const time_t write_timeout_sec,
                                       const time_t write_timeout_usec,
                                       T callback) -> bool {
            return process_server_socket_core(svr_sock,
                                              sock,
                                              keep_alive_max_count,
                                              keep_alive_timeout_sec,
                                              [&](bool close_connection, bool& connection_closed) {
                                                  SSLSocketStream strm(sock,
                                                                       ssl,
                                                                       read_timeout_sec,
                                                                       read_timeout_usec,
                                                                       write_timeout_sec,
                                                                       write_timeout_usec);
                                                  return callback(strm, close_connection, connection_closed);
                                              });
        }

        template <typename T>
        auto process_client_socket_ssl(SSL* ssl,
                                       const socket_t sock,
                                       const time_t read_timeout_sec,
                                       const time_t read_timeout_usec,
                                       const time_t write_timeout_sec,
                                       const time_t write_timeout_usec,
                                       const time_t max_timeout_msec,
                                       const std::chrono::time_point<std::chrono::steady_clock> start_time,
                                       const T& callback) -> bool {
            SSLSocketStream strm(sock, ssl, read_timeout_sec, read_timeout_usec, write_timeout_sec, write_timeout_usec, max_timeout_msec, start_time);
            return callback(strm);
        }

        // SSL socket stream implementation
        inline SSLSocketStream::SSLSocketStream(const socket_t sock,
                                                SSL* ssl,
                                                const time_t read_timeout_sec,
                                                const time_t read_timeout_usec,
                                                const time_t write_timeout_sec,
                                                const time_t write_timeout_usec,
                                                const time_t max_timeout_msec,
                                                const std::chrono::time_point<std::chrono::steady_clock> start_time) : sock_(sock),
            ssl_(ssl),
            read_timeout_sec_(read_timeout_sec),
            read_timeout_usec_(read_timeout_usec),
            write_timeout_sec_(write_timeout_sec),
            write_timeout_usec_(write_timeout_usec),
            max_timeout_msec_(max_timeout_msec),
            start_time(start_time) { SSL_clear_mode(ssl, SSL_MODE_AUTO_RETRY); }

        inline SSLSocketStream::~SSLSocketStream() = default;

        inline auto SSLSocketStream::is_readable() const -> bool {
            if (max_timeout_msec_ <= 0)
                return select_read(sock_, read_timeout_sec_, read_timeout_usec_) > 0;

            time_t read_timeout_sec;
            time_t read_timeout_usec;
            calc_actual_timeout(max_timeout_msec_, duration(), read_timeout_sec_, read_timeout_usec_, read_timeout_sec, read_timeout_usec);

            return select_read(sock_, read_timeout_sec, read_timeout_usec) > 0;
        }

        inline auto SSLSocketStream::is_writable() const -> bool {
            return select_write(sock_, write_timeout_sec_, write_timeout_usec_) > 0 && is_socket_alive(sock_) && !
                is_ssl_peer_could_be_closed(ssl_, sock_);
        }

        inline auto SSLSocketStream::read(char* ptr, const size_t size) -> ssize_t {
            if (SSL_pending(ssl_) > 0)
                return SSL_read(ssl_, ptr, static_cast<int>(size));
            if (is_readable()) {
                auto ret = SSL_read(ssl_, ptr, static_cast<int>(size));
                if (ret < 0) {
                    auto err = SSL_get_error(ssl_, ret);
                    auto n = 1000;
#ifdef _WIN32
                    while (--n >= 0 && (err == SSL_ERROR_WANT_READ || (err == SSL_ERROR_SYSCALL && WSAGetLastError() == WSAETIMEDOUT))) {
#else
      while (--n >= 0 && err == SSL_ERROR_WANT_READ) {
#endif
                        if (SSL_pending(ssl_) > 0)
                            return SSL_read(ssl_, ptr, static_cast<int>(size));
                        if (is_readable()) {
                            std::this_thread::sleep_for(std::chrono::microseconds{
                                10
                            });
                            ret = SSL_read(ssl_, ptr, static_cast<int>(size));
                            if (ret >= 0)
                                return ret;
                            err = SSL_get_error(ssl_, ret);
                        } else
                            return -1;
                    }
                }
                return ret;
            }
            return -1;
        }

        inline auto SSLSocketStream::write(const char* ptr, const size_t size) -> ssize_t {
            if (is_writable()) {
                const auto handle_size = static_cast<int>(std::min<size_t>(size, (std::numeric_limits<int>::max)()));

                auto ret = SSL_write(ssl_, ptr, handle_size);
                if (ret < 0) {
                    auto err = SSL_get_error(ssl_, ret);
                    auto n = 1000;
#ifdef _WIN32
                    while (--n >= 0 && (err == SSL_ERROR_WANT_WRITE || (err == SSL_ERROR_SYSCALL && WSAGetLastError() == WSAETIMEDOUT))) {
#else
      while (--n >= 0 && err == SSL_ERROR_WANT_WRITE) {
#endif
                        if (is_writable()) {
                            std::this_thread::sleep_for(std::chrono::microseconds{
                                10
                            });
                            ret = SSL_write(ssl_, ptr, handle_size);
                            if (ret >= 0)
                                return ret;
                            err = SSL_get_error(ssl_, ret);
                        } else
                            return -1;
                    }
                }
                return ret;
            }
            return -1;
        }

        inline auto SSLSocketStream::get_remote_ip_and_port(std::string& ip, int& port) const -> void {
            detail::get_remote_ip_and_port(sock_, ip, port);
        }

        inline auto SSLSocketStream::get_local_ip_and_port(std::string& ip, int& port) const -> void {
            detail::get_local_ip_and_port(sock_, ip, port);
        }

        inline auto SSLSocketStream::socket() const -> socket_t { return sock_; }

        inline auto SSLSocketStream::duration() const -> time_t {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();
        }
    } // namespace detail

    // SSL HTTP server implementation
    inline SSLServer::SSLServer(const char* cert_path,
                                const char* private_key_path,
                                const char* client_ca_cert_file_path,
                                const char* client_ca_cert_dir_path,
                                const char* private_key_password) {
        ctx_ = SSL_CTX_new(TLS_server_method());

        if (ctx_) {
            SSL_CTX_set_options(ctx_, SSL_OP_NO_COMPRESSION | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

            SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);

            if (private_key_password != nullptr && private_key_password[0] != '\0')
                SSL_CTX_set_default_passwd_cb_userdata(ctx_, const_cast<char*>(private_key_password));

            if (SSL_CTX_use_certificate_chain_file(ctx_, cert_path) != 1 || SSL_CTX_use_PrivateKey_file(ctx_, private_key_path, SSL_FILETYPE_PEM) != 1
                ||
                SSL_CTX_check_private_key(ctx_) != 1) {
                SSL_CTX_free(ctx_);
                ctx_ = nullptr;
            } else if (client_ca_cert_file_path || client_ca_cert_dir_path) {
                SSL_CTX_load_verify_locations(ctx_, client_ca_cert_file_path, client_ca_cert_dir_path);

                SSL_CTX_set_verify(ctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
            }
        }
    }

    inline SSLServer::SSLServer(X509* cert, EVP_PKEY* private_key, X509_STORE* client_ca_cert_store) {
        ctx_ = SSL_CTX_new(TLS_server_method());

        if (ctx_) {
            SSL_CTX_set_options(ctx_, SSL_OP_NO_COMPRESSION | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

            SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);

            if (SSL_CTX_use_certificate(ctx_, cert) != 1 || SSL_CTX_use_PrivateKey(ctx_, private_key) != 1) {
                SSL_CTX_free(ctx_);
                ctx_ = nullptr;
            } else if (client_ca_cert_store) {
                SSL_CTX_set_cert_store(ctx_, client_ca_cert_store);

                SSL_CTX_set_verify(ctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
            }
        }
    }

    inline SSLServer::SSLServer(const std::function<bool(SSL_CTX& ssl_ctx)>& setup_ssl_ctx_callback) {
        ctx_ = SSL_CTX_new(TLS_method());
        if (ctx_) {
            if (!setup_ssl_ctx_callback(*ctx_)) {
                SSL_CTX_free(ctx_);
                ctx_ = nullptr;
            }
        }
    }

    inline SSLServer::~SSLServer() {
        if (ctx_)
            SSL_CTX_free(ctx_);
    }

    inline auto SSLServer::is_valid() const -> bool { return ctx_; }

    inline auto SSLServer::ssl_context() const -> SSL_CTX* { return ctx_; }

    inline auto SSLServer::update_certs(X509* cert, EVP_PKEY* private_key, X509_STORE* client_ca_cert_store) -> void {
        std::lock_guard guard(ctx_mutex_);

        SSL_CTX_use_certificate(ctx_, cert);
        SSL_CTX_use_PrivateKey(ctx_, private_key);

        if (client_ca_cert_store != nullptr)
            SSL_CTX_set_cert_store(ctx_, client_ca_cert_store);
    }

    inline auto SSLServer::process_and_close_socket(const socket_t sock, const std::string& ip, const int port) -> bool {
        const auto ssl = detail::ssl_new(sock,
                                         ctx_,
                                         ctx_mutex_,
                                         [&](SSL* ssl2) {
                                             return detail::ssl_connect_or_accept_nonblocking(
                                                 sock,
                                                 ssl2,
                                                 SSL_accept,
                                                 read_timeout_sec_,
                                                 read_timeout_usec_);
                                         },
                                         [](SSL* /*ssl2*/) { return true; });

        auto ret = false;
        if (ssl) {
            std::string remote_addr;
            int remote_port = 0;
            if (ip.empty())
                detail::get_remote_ip_and_port(sock, remote_addr, remote_port);
            else {
                remote_addr = ip;
                remote_port = port;
            }

            std::string local_addr;
            int local_port = 0;
            detail::get_local_ip_and_port(sock, local_addr, local_port);

            ret = detail::process_server_socket_ssl(svr_sock_,
                                                    ssl,
                                                    sock,
                                                    keep_alive_max_count_,
                                                    keep_alive_timeout_sec_,
                                                    read_timeout_sec_,
                                                    read_timeout_usec_,
                                                    write_timeout_sec_,
                                                    write_timeout_usec_,
                                                    [&](Stream& strm, const bool close_connection, bool& connection_closed) {
                                                        return process_request(strm,
                                                                               remote_addr,
                                                                               remote_port,
                                                                               local_addr,
                                                                               local_port,
                                                                               close_connection,
                                                                               connection_closed,
                                                                               [&](Request& req) { req.ssl = ssl; });
                                                    });

            // Shutdown gracefully if the result seemed successful, non-gracefully if
            // the connection appeared to be closed.
            const bool shutdown_gracefully = ret;
            detail::ssl_delete(ctx_mutex_, ssl, sock, shutdown_gracefully);
        }

        IOCPSocket::close_client(sock);
        return ret;
    }

    // SSL HTTP client implementation
    inline SSLClient::SSLClient(const std::string& host) : SSLClient(host, 443, std::string(), std::string()) {}

    inline SSLClient::SSLClient(const std::string& host, const int port) : SSLClient(host, port, std::string(), std::string()) {}

    inline SSLClient::SSLClient(const std::string& host,
                                const int port,
                                const std::string& client_cert_path,
                                const std::string& client_key_path,
                                const std::string& private_key_password) : ClientImpl(host, port, client_cert_path, client_key_path) {
        ctx_ = SSL_CTX_new(TLS_client_method());

        SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);

        detail::split(host_.data(),
                      &host_[host_.size()],
                      '.',
                      [&](const char* b, const char* e) { host_components_.emplace_back(b, e); });

        if (!client_cert_path.empty() && !client_key_path.empty()) {
            if (!private_key_password.empty())
                SSL_CTX_set_default_passwd_cb_userdata(ctx_, const_cast<char*>(private_key_password.c_str()));

            if (SSL_CTX_use_certificate_file(ctx_, client_cert_path.c_str(), SSL_FILETYPE_PEM) != 1 || SSL_CTX_use_PrivateKey_file(
                    ctx_,
                    client_key_path.c_str(),
                    SSL_FILETYPE_PEM)
                != 1) {
                SSL_CTX_free(ctx_);
                ctx_ = nullptr;
            }
        }
    }

    inline SSLClient::SSLClient(const std::string& host,
                                const int port,
                                X509* client_cert,
                                EVP_PKEY* client_key,
                                const std::string& private_key_password) : ClientImpl(host, port) {
        ctx_ = SSL_CTX_new(TLS_client_method());

        detail::split(host_.data(),
                      &host_[host_.size()],
                      '.',
                      [&](const char* b, const char* e) { host_components_.emplace_back(b, e); });

        if (client_cert != nullptr && client_key != nullptr) {
            if (!private_key_password.empty())
                SSL_CTX_set_default_passwd_cb_userdata(ctx_, const_cast<char*>(private_key_password.c_str()));

            if (SSL_CTX_use_certificate(ctx_, client_cert) != 1 || SSL_CTX_use_PrivateKey(ctx_, client_key) != 1) {
                SSL_CTX_free(ctx_);
                ctx_ = nullptr;
            }
        }
    }

    inline SSLClient::~SSLClient() {
        if (ctx_)
            SSL_CTX_free(ctx_);
        // Make sure to shut down SSL since shutdown_ssl will resolve to the
        // base function rather than the derived function once we get to the
        // base class destructor, and won't free the SSL (causing a leak).
        shutdown_ssl_impl(socket_, true);
    }

    inline auto SSLClient::is_valid() const -> bool { return ctx_; }

    inline auto SSLClient::set_ca_cert_store(X509_STORE* ca_cert_store) const -> void {
        if (ca_cert_store) {
            if (ctx_) {
                if (SSL_CTX_get_cert_store(ctx_) != ca_cert_store) {
                    // Free memory allocated for old cert and use new store `ca_cert_store`
                    SSL_CTX_set_cert_store(ctx_, ca_cert_store);
                }
            } else
                X509_STORE_free(ca_cert_store);
        }
    }

    inline auto SSLClient::load_ca_cert_store(const char* ca_cert, const std::size_t size) const -> void {
        set_ca_cert_store(create_ca_cert_store(ca_cert, size));
    }

    inline auto SSLClient::get_openssl_verify_result() const -> long { return verify_result_; }

    inline auto SSLClient::ssl_context() const -> SSL_CTX* { return ctx_; }

    inline auto SSLClient::create_and_connect_socket(Socket& socket, Error& error) -> bool {
        return is_valid() && ClientImpl::create_and_connect_socket(socket, error);
    }

    // Assumes that socket_mutex_ is locked and that there are no requests in flight
    inline auto SSLClient::connect_with_proxy(Socket& socket,
                                              const std::chrono::time_point<std::chrono::steady_clock> start_time,
                                              Response& res,
                                              bool& success,
                                              Error& error) -> bool {
        success = true;
        Response proxy_res;
        if (!detail::process_client_socket(socket.sock,
                                           read_timeout_sec_,
                                           read_timeout_usec_,
                                           write_timeout_sec_,
                                           write_timeout_usec_,
                                           max_timeout_msec_,
                                           start_time,
                                           [&](Stream& strm) {
                                               Request req2;
                                               req2.method = "CONNECT";
                                               req2.path = host_and_port_;
                                               if (max_timeout_msec_ > 0)
                                                   req2.start_time_ = std::chrono::steady_clock::now();
                                               return process_request(strm, req2, proxy_res, false, error);
                                           })) {
            // Thread-safe to close everything because we are assuming there are no
            // requests in flight
            shutdown_ssl(socket, true);
            shutdown_socket(socket);
            close_socket(socket);
            success = false;
            return false;
        }

        if (proxy_res.status == ProxyAuthenticationRequired_407) {
            if (!proxy_digest_auth_username_.empty() && !proxy_digest_auth_password_.empty()) {
                std::map<std::string, std::string> auth;
                if (detail::parse_www_authenticate(proxy_res, auth, true)) {
                    proxy_res = Response();
                    if (!detail::process_client_socket(socket.sock,
                                                       read_timeout_sec_,
                                                       read_timeout_usec_,
                                                       write_timeout_sec_,
                                                       write_timeout_usec_,
                                                       max_timeout_msec_,
                                                       start_time,
                                                       [&](Stream& strm) {
                                                           Request req3;
                                                           req3.method = "CONNECT";
                                                           req3.path = host_and_port_;
                                                           req3.headers.insert(detail::make_digest_authentication_header(
                                                               req3,
                                                               auth,
                                                               1,
                                                               detail::random_string(10),
                                                               proxy_digest_auth_username_,
                                                               proxy_digest_auth_password_,
                                                               true));
                                                           if (max_timeout_msec_ > 0)
                                                               req3.start_time_ = std::chrono::steady_clock::now();
                                                           return process_request(strm, req3, proxy_res, false, error);
                                                       })) {
                        // Thread-safe to close everything because we are assuming there are
                        // no requests in flight
                        shutdown_ssl(socket, true);
                        shutdown_socket(socket);
                        close_socket(socket);
                        success = false;
                        return false;
                    }
                }
            }
        }

        // If status code is not 200, proxy request is failed.
        // Set error to ProxyConnection and return proxy response
        // as the response of the request
        if (proxy_res.status != OK_200) {
            error = Error::ProxyConnection;
            res = std::move(proxy_res);
            // Thread-safe to close everything because we are assuming there are
            // no requests in flight
            shutdown_ssl(socket, true);
            shutdown_socket(socket);
            close_socket(socket);
            return false;
        }

        return true;
    }

    inline auto SSLClient::load_certs() -> bool {
        auto ret = true;

        std::call_once(initialize_cert_,
                       [&] {
                           std::lock_guard guard(ctx_mutex_);
                           if (!ca_cert_file_path_.empty()) {
                               if (!SSL_CTX_load_verify_locations(ctx_, ca_cert_file_path_.c_str(), nullptr))
                                   ret = false;
                           } else if (!ca_cert_dir_path_.empty()) {
                               if (!SSL_CTX_load_verify_locations(ctx_, nullptr, ca_cert_dir_path_.c_str()))
                                   ret = false;
                           } else { const auto loaded = detail::load_system_certs_on_windows(SSL_CTX_get_cert_store(ctx_));
                               if (!loaded)
                                   SSL_CTX_set_default_verify_paths(ctx_);
                           }
                       });

        return ret;
    }

    inline auto SSLClient::initialize_ssl(Socket& socket, Error& error) -> bool {
        const auto ssl = detail::ssl_new(socket.sock,
                                         ctx_,
                                         ctx_mutex_,
                                         [&](SSL* ssl2) {
                                             if (server_certificate_verification_) {
                                                 if (!load_certs()) {
                                                     error = Error::SSLLoadingCerts;
                                                     return false;
                                                 }
                                                 SSL_set_verify(ssl2, SSL_VERIFY_NONE, nullptr);
                                             }

                                             if (!detail::ssl_connect_or_accept_nonblocking(
                                                 socket.sock,
                                                 ssl2,
                                                 SSL_connect,
                                                 connection_timeout_sec_,
                                                 connection_timeout_usec_)) {
                                                 error = Error::SSLConnection;
                                                 return false;
                                             }

                                             if (server_certificate_verification_) {
                                                 auto verification_status = SSLVerifierResponse::NoDecisionMade;

                                                 if (server_certificate_verifier_)
                                                     verification_status = server_certificate_verifier_(ssl2);

                                                 if (verification_status == SSLVerifierResponse::CertificateRejected) {
                                                     error = Error::SSLServerVerification;
                                                     return false;
                                                 }

                                                 if (verification_status == SSLVerifierResponse::NoDecisionMade) {
                                                     verify_result_ = SSL_get_verify_result(ssl2);

                                                     if (verify_result_ != X509_V_OK) {
                                                         error = Error::SSLServerVerification;
                                                         return false;
                                                     }

                                                     const auto server_cert = SSL_get1_peer_certificate(ssl2);
                                                     auto se = detail::scope_exit([&] { X509_free(server_cert); });

                                                     if (server_cert == nullptr) {
                                                         error = Error::SSLServerVerification;
                                                         return false;
                                                     }

                                                     if (server_hostname_verification_) {
                                                         if (!verify_host(server_cert)) {
                                                             error = Error::SSLServerHostnameVerification;
                                                             return false;
                                                         }
                                                     }
                                                 }
                                             }

                                             return true;
                                         },
                                         [&](SSL* ssl2) {
#if defined(OPENSSL_IS_BORINGSSL)
        SSL_set_tlsext_host_name(ssl2, host_.c_str());
#else
                                             // NOTE: Direct call instead of using the OpenSSL macro to suppress
                                             // -Wold-style-cast warning
                                             SSL_ctrl(ssl2,
                                                      SSL_CTRL_SET_TLSEXT_HOSTNAME,
                                                      TLSEXT_NAMETYPE_host_name,
                                                      const_cast<char*>(host_.c_str()));
#endif
                                             return true;
                                         });

        if (ssl) {
            socket.ssl = ssl;
            return true;
        }

        shutdown_socket(socket);
        close_socket(socket);
        return false;
    }

    inline auto SSLClient::shutdown_ssl(Socket& socket, const bool shutdown_gracefully) -> void { shutdown_ssl_impl(socket, shutdown_gracefully); }

    inline auto SSLClient::shutdown_ssl_impl(Socket& socket, const bool shutdown_gracefully) -> void {
        if (socket.sock == INVALID_SOCKET) {
            assert(socket.ssl == nullptr);
            return;
        }
        if (socket.ssl) {
            detail::ssl_delete(ctx_mutex_, socket.ssl, socket.sock, shutdown_gracefully);
            socket.ssl = nullptr;
        }
        assert(socket.ssl == nullptr);
    }

    inline auto SSLClient::process_socket(const Socket& socket,
                                          const std::chrono::time_point<std::chrono::steady_clock> start_time,
                                          const std::function<bool(Stream& strm)> callback) -> bool {
        assert(socket.ssl);
        return detail::process_client_socket_ssl(socket.ssl,
                                                 socket.sock,
                                                 read_timeout_sec_,
                                                 read_timeout_usec_,
                                                 write_timeout_sec_,
                                                 write_timeout_usec_,
                                                 max_timeout_msec_,
                                                 start_time,
                                                 callback);
    }

    inline auto SSLClient::is_ssl() const -> bool { return true; }

    inline auto SSLClient::verify_host(X509* server_cert) const -> bool {
        /* Quote from RFC2818 section 3.1 "Server Identity"
      
           If a subjectAltName extension of type dNSName is present, that MUST
           be used as the identity. Otherwise, the (most specific) Common Name
           field in the Subject field of the certificate MUST be used. Although
           the use of the Common Name is existing practice, it is deprecated and
           Certification Authorities are encouraged to use the dNSName instead.
      
           Matching is performed using the matching rules specified by
           [RFC2459].  If more than one identity of a given type is present in
           the certificate (e.g., more than one dNSName name, a match in any one
           of the set is considered acceptable.) Names may contain the wildcard
           character * which is considered to match any single domain name
           component or component fragment. E.g., *.a.com matches foo.a.com but
           not bar.foo.a.com. f*.com matches foo.com but not bar.com.
      
           In some cases, the URI is specified as an IP address rather than a
           hostname. In this case, the iPAddress subjectAltName must be present
           in the certificate and must exactly match the IP in the URI.
      
        */
        return verify_host_with_subject_alt_name(server_cert) || verify_host_with_common_name(server_cert);
    }

    inline auto SSLClient::verify_host_with_subject_alt_name(const X509* server_cert) const -> bool {
        auto ret = false;

        auto type = GEN_DNS;

        in6_addr addr6{};
        in_addr addr{};
        size_t addr_len = 0;

#ifndef __MINGW32__
        if (inet_pton(AF_INET6, host_.c_str(), &addr6)) {
            type = GEN_IPADD;
            addr_len = sizeof(in6_addr);
        } else if (inet_pton(AF_INET, host_.c_str(), &addr)) {
            type = GEN_IPADD;
            addr_len = sizeof(in_addr);
        }
#endif

        const auto alt_names = static_cast<const struct stack_st_GENERAL_NAME*>(
            X509_get_ext_d2i(server_cert, NID_subject_alt_name, nullptr, nullptr));

        if (alt_names) {
            auto dsn_matched = false;
            auto ip_matched = false;

            auto count = sk_GENERAL_NAME_num(alt_names);

            for (decltype(count) i = 0; i < count && !dsn_matched; i++) {
                const auto val = sk_GENERAL_NAME_value(alt_names, i);
                if (val->type == type) {
                    const auto name = reinterpret_cast<const char*>(ASN1_STRING_get0_data(val->d.ia5));
                    const auto name_len = static_cast<size_t>(ASN1_STRING_length(val->d.ia5));

                    switch (type) {
                        case GEN_DNS:
                            dsn_matched = check_host_name(name, name_len);
                            break;

                        case GEN_IPADD:
                            if (!memcmp(&addr6, name, addr_len) || !memcmp(&addr, name, addr_len))
                                ip_matched = true;
                            break;
                    }
                }
            }

            if (dsn_matched || ip_matched)
                ret = true;
        }

        GENERAL_NAMES_free(const_cast<STACK_OF(GENERAL_NAME)*>(alt_names));
        return ret;
    }

    inline auto SSLClient::verify_host_with_common_name(const X509* server_cert) const -> bool {
        const auto subject_name = X509_get_subject_name(server_cert);

        if (subject_name != nullptr) {
            char name[BUFSIZ];
            const auto name_len = X509_NAME_get_text_by_NID(subject_name, NID_commonName, name, sizeof(name));

            if (name_len != -1)
                return check_host_name(name, static_cast<size_t>(name_len));
        }

        return false;
    }

    inline auto SSLClient::check_host_name(const char* pattern, const size_t pattern_len) const -> bool {
        if (host_.size() == pattern_len && host_ == pattern)
            return true;

        // Wildcard match
        // https://bugs.launchpad.net/ubuntu/+source/firefox-3.0/+bug/376484
        std::vector<std::string> pattern_components;
        detail::split(&pattern[0],
                      &pattern[pattern_len],
                      '.',
                      [&](const char* b, const char* e) { pattern_components.emplace_back(b, e); });

        if (host_components_.size() != pattern_components.size())
            return false;

        auto itr = pattern_components.begin();
        for (const auto& h : host_components_) {
            auto& p = *itr;
            if (p != h && p != "*") {
                const auto partial_match = p.size() > 0 && p[p.size() - 1] == '*' && !p.compare(0, p.size() - 1, h);
                if (!partial_match)
                    return false;
            }
            ++itr;
        }

        return true;
    }
#endif

    // Universal client implementation
    inline Client::Client(const std::string& scheme_host_port) : Client(scheme_host_port, std::string(), std::string()) {}

    inline Client::Client(const std::string& scheme_host_port, const std::string& client_cert_path, const std::string& client_key_path) {
        const static std::regex re(R"((?:([a-z]+):\/\/)?(?:\[([a-fA-F\d:]+)\]|([^:/?#]+))(?::(\d+))?)");

        std::smatch m;
        if (std::regex_match(scheme_host_port, m, re)) {
            auto scheme = m[1].str();

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
            if (!scheme.empty() && scheme != "http" && scheme != "https") {
#else
    if (!scheme.empty() && scheme != "http") {
#endif
#ifndef CPPHTTPLIB_NO_EXCEPTIONS
                const std::string msg = "'" + scheme + "' scheme is not supported.";
                throw std::invalid_argument(msg);
#endif
            }

            const auto is_ssl = scheme == "https";

            auto host = m[2].str();
            if (host.empty()) {
                host = m[3].str();
            }

            const auto port_str = m[4].str();
            auto port = !port_str.empty() ? std::stoi(port_str) : is_ssl ? 443 : 80;

            if (is_ssl) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
                cli_ = detail::make_unique<SSLClient>(host, port, client_cert_path, client_key_path);
                is_ssl_ = is_ssl;
#endif
            } else {
                cli_ = detail::make_unique<ClientImpl>(host, port, client_cert_path, client_key_path);
            }
        } else {
            // NOTE: Update TEST(UniversalClientImplTest, Ipv6LiteralAddress)
            // if port param below changes.
            cli_ = detail::make_unique<ClientImpl>(scheme_host_port, 80, client_cert_path, client_key_path);
        }
    } // namespace detail

    inline Client::Client(const std::string& host, int port) : cli_(detail::make_unique<ClientImpl>(host, port)) {}

    inline Client::Client(const std::string& host, int port, const std::string& client_cert_path, const std::string& client_key_path) : cli_(
        detail::make_unique<ClientImpl>(host, port, client_cert_path, client_key_path)) {}

    inline Client::~Client() = default;

    inline auto Client::is_valid() const -> bool { return cli_ != nullptr && cli_->is_valid(); }

    inline auto Client::Get(const std::string& path) const -> Result { return cli_->Get(path); }

    inline auto Client::Get(const std::string& path, const Headers& headers) const -> Result { return cli_->Get(path, headers); }

    inline auto Client::Get(const std::string& path, Progress progress) const -> Result { return cli_->Get(path, std::move(progress)); }

    inline auto Client::Get(const std::string& path, const Headers& headers, Progress progress) const -> Result {
        return cli_->Get(path, headers, std::move(progress));
    }

    inline auto Client::Get(const std::string& path, const ContentReceiver& content_receiver) const -> Result {
        return cli_->Get(path, content_receiver);
    }

    inline auto Client::Get(const std::string& path, const Headers& headers, const ContentReceiver& content_receiver) const -> Result {
        return cli_->Get(path, headers, content_receiver);
    }

    inline auto Client::Get(const std::string& path, const ContentReceiver& content_receiver, Progress progress) const -> Result {
        return cli_->Get(path, content_receiver, std::move(progress));
    }

    inline auto Client::Get(const std::string& path,
                            const Headers& headers,
                            const ContentReceiver& content_receiver,
                            Progress progress) const -> Result { return cli_->Get(path, headers, content_receiver, std::move(progress)); }

    inline auto Client::Get(const std::string& path, ResponseHandler response_handler, const ContentReceiver& content_receiver) const -> Result {
        return cli_->Get(path, std::move(response_handler), content_receiver);
    }

    inline auto Client::Get(const std::string& path,
                            const Headers& headers,
                            ResponseHandler response_handler,
                            const ContentReceiver& content_receiver) const -> Result {
        return cli_->Get(path, headers, std::move(response_handler), content_receiver);
    }

    inline auto Client::Get(const std::string& path,
                            ResponseHandler response_handler,
                            const ContentReceiver& content_receiver,
                            Progress progress) const -> Result {
        return cli_->Get(path, std::move(response_handler), content_receiver, std::move(progress));
    }

    inline auto Client::Get(const std::string& path,
                            const Headers& headers,
                            ResponseHandler response_handler,
                            const ContentReceiver& content_receiver,
                            Progress progress) const -> Result {
        return cli_->Get(path, headers, std::move(response_handler), content_receiver, std::move(progress));
    }

    inline auto Client::Get(const std::string& path, const Params& params, const Headers& headers, Progress progress) const -> Result {
        return cli_->Get(path, params, headers, std::move(progress));
    }

    inline auto Client::Get(const std::string& path,
                            const Params& params,
                            const Headers& headers,
                            const ContentReceiver& content_receiver,
                            Progress progress) const -> Result {
        return cli_->Get(path, params, headers, content_receiver, std::move(progress));
    }

    inline auto Client::Get(const std::string& path,
                            const Params& params,
                            const Headers& headers,
                            ResponseHandler response_handler,
                            const ContentReceiver& content_receiver,
                            Progress progress) const -> Result {
        return cli_->Get(path, params, headers, std::move(response_handler), content_receiver, std::move(progress));
    }

    inline auto Client::Head(const std::string& path) const -> Result { return cli_->Head(path); }

    inline auto Client::Head(const std::string& path, const Headers& headers) const -> Result { return cli_->Head(path, headers); }

    inline auto Client::Post(const std::string& path) const -> Result { return cli_->Post(path); }

    inline auto Client::Post(const std::string& path, const Headers& headers) const -> Result { return cli_->Post(path, headers); }

    inline auto Client::Post(const std::string& path,
                             const char* body,
                             const size_t content_length,
                             const std::string& content_type) const -> Result { return cli_->Post(path, body, content_length, content_type); }

    inline auto Client::Post(const std::string& path,
                             const Headers& headers,
                             const char* body,
                             const size_t content_length,
                             const std::string& content_type) const -> Result {
        return cli_->Post(path, headers, body, content_length, content_type);
    }

    inline auto Client::Post(const std::string& path,
                             const Headers& headers,
                             const char* body,
                             const size_t content_length,
                             const std::string& content_type,
                             const Progress& progress) const -> Result {
        return cli_->Post(path, headers, body, content_length, content_type, progress);
    }

    inline auto Client::Post(const std::string& path, const std::string& body, const std::string& content_type) const -> Result {
        return cli_->Post(path, body, content_type);
    }

    inline auto Client::Post(const std::string& path,
                             const std::string& body,
                             const std::string& content_type,
                             const Progress& progress) const -> Result { return cli_->Post(path, body, content_type, progress); }

    inline auto Client::Post(const std::string& path,
                             const Headers& headers,
                             const std::string& body,
                             const std::string& content_type) const -> Result { return cli_->Post(path, headers, body, content_type); }

    inline auto Client::Post(const std::string& path,
                             const Headers& headers,
                             const std::string& body,
                             const std::string& content_type,
                             const Progress& progress) const -> Result { return cli_->Post(path, headers, body, content_type, progress); }

    inline auto Client::Post(const std::string& path,
                             const size_t content_length,
                             ContentProvider content_provider,
                             const std::string& content_type) const -> Result {
        return cli_->Post(path, content_length, std::move(content_provider), content_type);
    }

    inline auto Client::Post(const std::string& path,
                             ContentProviderWithoutLength content_provider,
                             const std::string& content_type) const -> Result { return cli_->Post(path, std::move(content_provider), content_type); }

    inline auto Client::Post(const std::string& path,
                             const Headers& headers,
                             const size_t content_length,
                             ContentProvider content_provider,
                             const std::string& content_type) const -> Result {
        return cli_->Post(path, headers, content_length, std::move(content_provider), content_type);
    }

    inline auto Client::Post(const std::string& path,
                             const Headers& headers,
                             ContentProviderWithoutLength content_provider,
                             const std::string& content_type) const -> Result {
        return cli_->Post(path, headers, std::move(content_provider), content_type);
    }

    inline auto Client::Post(const std::string& path, const Params& params) const -> Result { return cli_->Post(path, params); }

    inline auto Client::Post(const std::string& path, const Headers& headers, const Params& params) const -> Result {
        return cli_->Post(path, headers, params);
    }

    inline auto Client::Post(const std::string& path, const Headers& headers, const Params& params, const Progress& progress) const -> Result {
        return cli_->Post(path, headers, params, progress);
    }

    inline auto Client::Post(const std::string& path, const MultipartFormDataItems& items) const -> Result { return cli_->Post(path, items); }

    inline auto Client::Post(const std::string& path, const Headers& headers, const MultipartFormDataItems& items) const -> Result {
        return cli_->Post(path, headers, items);
    }

    inline auto Client::Post(const std::string& path,
                             const Headers& headers,
                             const MultipartFormDataItems& items,
                             const std::string& boundary) const -> Result { return cli_->Post(path, headers, items, boundary); }

    inline auto Client::Post(const std::string& path,
                             const Headers& headers,
                             const MultipartFormDataItems& items,
                             const MultipartFormDataProviderItems& provider_items) const -> Result {
        return cli_->Post(path, headers, items, provider_items);
    }

    inline auto Client::Put(const std::string& path) const -> Result { return cli_->Put(path); }

    inline auto Client::Put(const std::string& path, const char* body, const size_t content_length, const std::string& content_type) const -> Result {
        return cli_->Put(path, body, content_length, content_type);
    }

    inline auto Client::Put(const std::string& path,
                            const Headers& headers,
                            const char* body,
                            const size_t content_length,
                            const std::string& content_type) const -> Result { return cli_->Put(path, headers, body, content_length, content_type); }

    inline auto Client::Put(const std::string& path,
                            const Headers& headers,
                            const char* body,
                            const size_t content_length,
                            const std::string& content_type,
                            const Progress& progress) const -> Result {
        return cli_->Put(path, headers, body, content_length, content_type, progress);
    }

    inline auto Client::Put(const std::string& path, const std::string& body, const std::string& content_type) const -> Result {
        return cli_->Put(path, body, content_type);
    }

    inline auto Client::Put(const std::string& path,
                            const std::string& body,
                            const std::string& content_type,
                            const Progress& progress) const -> Result { return cli_->Put(path, body, content_type, progress); }

    inline auto Client::Put(const std::string& path,
                            const Headers& headers,
                            const std::string& body,
                            const std::string& content_type) const -> Result { return cli_->Put(path, headers, body, content_type); }

    inline auto Client::Put(const std::string& path,
                            const Headers& headers,
                            const std::string& body,
                            const std::string& content_type,
                            const Progress& progress) const -> Result { return cli_->Put(path, headers, body, content_type, progress); }

    inline auto Client::Put(const std::string& path,
                            const size_t content_length,
                            ContentProvider content_provider,
                            const std::string& content_type) const -> Result {
        return cli_->Put(path, content_length, std::move(content_provider), content_type);
    }

    inline auto Client::Put(const std::string& path, ContentProviderWithoutLength content_provider, const std::string& content_type) const -> Result {
        return cli_->Put(path, std::move(content_provider), content_type);
    }

    inline auto Client::Put(const std::string& path,
                            const Headers& headers,
                            const size_t content_length,
                            ContentProvider content_provider,
                            const std::string& content_type) const -> Result {
        return cli_->Put(path, headers, content_length, std::move(content_provider), content_type);
    }

    inline auto Client::Put(const std::string& path,
                            const Headers& headers,
                            ContentProviderWithoutLength content_provider,
                            const std::string& content_type) const -> Result {
        return cli_->Put(path, headers, std::move(content_provider), content_type);
    }

    inline auto Client::Put(const std::string& path, const Params& params) const -> Result { return cli_->Put(path, params); }

    inline auto Client::Put(const std::string& path, const Headers& headers, const Params& params) const -> Result {
        return cli_->Put(path, headers, params);
    }

    inline auto Client::Put(const std::string& path, const Headers& headers, const Params& params, const Progress& progress) const -> Result {
        return cli_->Put(path, headers, params, progress);
    }

    inline auto Client::Put(const std::string& path, const MultipartFormDataItems& items) const -> Result { return cli_->Put(path, items); }

    inline auto Client::Put(const std::string& path, const Headers& headers, const MultipartFormDataItems& items) const -> Result {
        return cli_->Put(path, headers, items);
    }

    inline auto Client::Put(const std::string& path,
                            const Headers& headers,
                            const MultipartFormDataItems& items,
                            const std::string& boundary) const -> Result { return cli_->Put(path, headers, items, boundary); }

    inline auto Client::Put(const std::string& path,
                            const Headers& headers,
                            const MultipartFormDataItems& items,
                            const MultipartFormDataProviderItems& provider_items) const -> Result {
        return cli_->Put(path, headers, items, provider_items);
    }

    inline auto Client::Patch(const std::string& path) const -> Result { return cli_->Patch(path); }

    inline auto Client::Patch(const std::string& path,
                              const char* body,
                              const size_t content_length,
                              const std::string& content_type) const -> Result { return cli_->Patch(path, body, content_length, content_type); }

    inline auto Client::Patch(const std::string& path,
                              const char* body,
                              const size_t content_length,
                              const std::string& content_type,
                              const Progress& progress) const -> Result { return cli_->Patch(path, body, content_length, content_type, progress); }

    inline auto Client::Patch(const std::string& path,
                              const Headers& headers,
                              const char* body,
                              const size_t content_length,
                              const std::string& content_type) const -> Result {
        return cli_->Patch(path, headers, body, content_length, content_type);
    }

    inline auto Client::Patch(const std::string& path,
                              const Headers& headers,
                              const char* body,
                              const size_t content_length,
                              const std::string& content_type,
                              const Progress& progress) const -> Result {
        return cli_->Patch(path, headers, body, content_length, content_type, progress);
    }

    inline auto Client::Patch(const std::string& path, const std::string& body, const std::string& content_type) const -> Result {
        return cli_->Patch(path, body, content_type);
    }

    inline auto Client::Patch(const std::string& path,
                              const std::string& body,
                              const std::string& content_type,
                              const Progress& progress) const -> Result { return cli_->Patch(path, body, content_type, progress); }

    inline auto Client::Patch(const std::string& path,
                              const Headers& headers,
                              const std::string& body,
                              const std::string& content_type) const -> Result { return cli_->Patch(path, headers, body, content_type); }

    inline auto Client::Patch(const std::string& path,
                              const Headers& headers,
                              const std::string& body,
                              const std::string& content_type,
                              const Progress& progress) const -> Result { return cli_->Patch(path, headers, body, content_type, progress); }

    inline auto Client::Patch(const std::string& path,
                              const size_t content_length,
                              ContentProvider content_provider,
                              const std::string& content_type) const -> Result {
        return cli_->Patch(path, content_length, std::move(content_provider), content_type);
    }

    inline auto Client::Patch(const std::string& path,
                              ContentProviderWithoutLength content_provider,
                              const std::string& content_type) const -> Result {
        return cli_->Patch(path, std::move(content_provider), content_type);
    }

    inline auto Client::Patch(const std::string& path,
                              const Headers& headers,
                              const size_t content_length,
                              ContentProvider content_provider,
                              const std::string& content_type) const -> Result {
        return cli_->Patch(path, headers, content_length, std::move(content_provider), content_type);
    }

    inline auto Client::Patch(const std::string& path,
                              const Headers& headers,
                              ContentProviderWithoutLength content_provider,
                              const std::string& content_type) const -> Result {
        return cli_->Patch(path, headers, std::move(content_provider), content_type);
    }

    inline auto Client::Delete(const std::string& path) const -> Result { return cli_->Delete(path); }

    inline auto Client::Delete(const std::string& path, const Headers& headers) const -> Result { return cli_->Delete(path, headers); }

    inline auto Client::Delete(const std::string& path,
                               const char* body,
                               const size_t content_length,
                               const std::string& content_type) const -> Result { return cli_->Delete(path, body, content_length, content_type); }

    inline auto Client::Delete(const std::string& path,
                               const char* body,
                               const size_t content_length,
                               const std::string& content_type,
                               const Progress& progress) const -> Result { return cli_->Delete(path, body, content_length, content_type, progress); }

    inline auto Client::Delete(const std::string& path,
                               const Headers& headers,
                               const char* body,
                               const size_t content_length,
                               const std::string& content_type) const -> Result {
        return cli_->Delete(path, headers, body, content_length, content_type);
    }

    inline auto Client::Delete(const std::string& path,
                               const Headers& headers,
                               const char* body,
                               const size_t content_length,
                               const std::string& content_type,
                               const Progress& progress) const -> Result {
        return cli_->Delete(path, headers, body, content_length, content_type, progress);
    }

    inline auto Client::Delete(const std::string& path, const std::string& body, const std::string& content_type) const -> Result {
        return cli_->Delete(path, body, content_type);
    }

    inline auto Client::Delete(const std::string& path,
                               const std::string& body,
                               const std::string& content_type,
                               const Progress& progress) const -> Result { return cli_->Delete(path, body, content_type, progress); }

    inline auto Client::Delete(const std::string& path,
                               const Headers& headers,
                               const std::string& body,
                               const std::string& content_type) const -> Result { return cli_->Delete(path, headers, body, content_type); }

    inline auto Client::Delete(const std::string& path,
                               const Headers& headers,
                               const std::string& body,
                               const std::string& content_type,
                               const Progress& progress) const -> Result { return cli_->Delete(path, headers, body, content_type, progress); }

    inline auto Client::Options(const std::string& path) const -> Result { return cli_->Options(path); }

    inline auto Client::Options(const std::string& path, const Headers& headers) const -> Result { return cli_->Options(path, headers); }

    inline auto Client::send(Request& req, Response& res, Error& error) const -> bool { return cli_->send(req, res, error); }

    inline auto Client::send(const Request& req) const -> Result { return cli_->send(req); }

    inline auto Client::stop() const -> void { cli_->stop(); }

    inline auto Client::host() const -> std::string { return cli_->host(); }

    inline auto Client::port() const -> int { return cli_->port(); }

    inline auto Client::is_socket_open() const -> size_t { return cli_->is_socket_open(); }

    inline auto Client::socket() const -> socket_t { return cli_->socket(); }

    inline auto Client::set_hostname_addr_map(std::map<std::string, std::string> addr_map) const -> void {
        cli_->set_hostname_addr_map(std::move(addr_map));
    }

    inline auto Client::set_default_headers(Headers headers) const -> void { cli_->set_default_headers(std::move(headers)); }

    inline auto Client::set_header_writer(const std::function<ssize_t(Stream&, Headers&)>& writer) const -> void { cli_->set_header_writer(writer); }

    inline auto Client::set_address_family(const int family) const -> void { cli_->set_address_family(family); }

    inline auto Client::set_tcp_nodelay(const bool on) const -> void { cli_->set_tcp_nodelay(on); }

    inline auto Client::set_socket_options(SocketOptions socket_options) const -> void { cli_->set_socket_options(std::move(socket_options)); }

    inline auto Client::set_connection_timeout(const time_t sec, const time_t usec) const -> void { cli_->set_connection_timeout(sec, usec); }

    inline auto Client::set_read_timeout(const time_t sec, const time_t usec) const -> void { cli_->set_read_timeout(sec, usec); }

    inline auto Client::set_write_timeout(const time_t sec, const time_t usec) const -> void { cli_->set_write_timeout(sec, usec); }

    inline auto Client::set_basic_auth(const std::string& username, const std::string& password) const -> void {
        cli_->set_basic_auth(username, password);
    }

    inline auto Client::set_bearer_token_auth(const std::string& token) const -> void { cli_->set_bearer_token_auth(token); }
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    inline auto Client::set_digest_auth(const std::string& username, const std::string& password) const -> void {
        cli_->set_digest_auth(username, password);
    }
#endif

    inline auto Client::set_keep_alive(const bool on) const -> void { cli_->set_keep_alive(on); }

    inline auto Client::set_follow_location(const bool on) const -> void { cli_->set_follow_location(on); }

    inline auto Client::set_url_encode(const bool on) const -> void { cli_->set_url_encode(on); }

    inline auto Client::set_compress(const bool on) const -> void { cli_->set_compress(on); }

    inline auto Client::set_decompress(const bool on) const -> void { cli_->set_decompress(on); }

    inline auto Client::set_interface(const std::string& intf) const -> void { cli_->set_interface(intf); }

    inline auto Client::set_proxy(const std::string& host, const int port) const -> void { cli_->set_proxy(host, port); }

    inline auto Client::set_proxy_basic_auth(const std::string& username, const std::string& password) const -> void {
        cli_->set_proxy_basic_auth(username, password);
    }

    inline auto Client::set_proxy_bearer_token_auth(const std::string& token) const -> void { cli_->set_proxy_bearer_token_auth(token); }
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    inline auto Client::set_proxy_digest_auth(const std::string& username, const std::string& password) const -> void {
        cli_->set_proxy_digest_auth(username, password);
    }
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    inline auto Client::enable_server_certificate_verification(const bool enabled) const -> void {
        cli_->enable_server_certificate_verification(enabled);
    }

    inline auto Client::enable_server_hostname_verification(const bool enabled) const -> void { cli_->enable_server_hostname_verification(enabled); }

    inline auto Client::set_server_certificate_verifier(std::function<SSLVerifierResponse(SSL* ssl)> verifier) const -> void {
        cli_->set_server_certificate_verifier(std::move(verifier));
    }
#endif

    inline auto Client::set_logger(Logger logger) const -> void { cli_->set_logger(std::move(logger)); }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    inline auto Client::set_ca_cert_path(const std::string& ca_cert_file_path, const std::string& ca_cert_dir_path) const -> void {
        cli_->set_ca_cert_path(ca_cert_file_path, ca_cert_dir_path);
    }

    inline auto Client::set_ca_cert_store(X509_STORE* ca_cert_store) const -> void {
        if (is_ssl_)
            dynamic_cast<SSLClient&>(*cli_).set_ca_cert_store(ca_cert_store);
        else
            cli_->set_ca_cert_store(ca_cert_store);
    }

    inline auto Client::load_ca_cert_store(const char* ca_cert, const std::size_t size) const -> void {
        set_ca_cert_store(cli_->create_ca_cert_store(ca_cert, size));
    }

    inline auto Client::get_openssl_verify_result() const -> long {
        if (is_ssl_)
            return dynamic_cast<SSLClient&>(*cli_).get_openssl_verify_result();
        return -1; // NOTE: -1 doesn't match any of X509_V_ERR_???
    }

    inline auto Client::ssl_context() const -> SSL_CTX* {
        if (is_ssl_)
            return dynamic_cast<SSLClient&>(*cli_).ssl_context();
        return nullptr;
    }
#endif

    // ----------------------------------------------------------------------------
} // namespace httplib

#if defined(_WIN32) && defined(CPPHTTPLIB_USE_POLL)
#undef poll
#endif

#endif // CPPHTTPLIB_HTTPLIB_H
