/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_NET_H
#define RIN_SDK_NET_H
#include "abi.h"
#include "bluetooth/hci_control.h"
#include "net/wifi_awdl_transport.h"
#ifdef __cplusplus
extern "C" {
#endif

/* `port` is host byte order. `address` contains network-order address bytes:
 * IPv4 uses the first 4 bytes and requires the rest to be zero; IPv6 uses all
 * 16 bytes. `scope_id` is IPv6-only and must be zero for IPv4. */
typedef struct RinSocketAddressV1 {
    uint32_t struct_size;
    uint32_t version;
    uint16_t family;
    uint16_t port;
    uint32_t scope_id;
    uint8_t address[16];
    uint64_t reserved[2];
} RinSocketAddressV1;

/* rin_socket_create_v1() domain, type, and protocol values. Address family
 * fields in RinSocketAddressV1 use the same INET/INET6 values. Socket flags
 * are ORed into the type; raw sockets currently support only IPv6 ICMP. */
enum {
    RIN_NET_DOMAIN_INET = UINT32_C(2),
    RIN_NET_DOMAIN_INET6 = UINT32_C(10),
    RIN_NET_SOCKET_TYPE_STREAM = UINT32_C(1),
    RIN_NET_SOCKET_TYPE_DGRAM = UINT32_C(2),
    RIN_NET_SOCKET_TYPE_RAW = UINT32_C(3),
    RIN_NET_SOCKET_FLAG_NONBLOCK = UINT32_C(0x00000800),
    RIN_NET_SOCKET_FLAG_CLOEXEC = UINT32_C(0x00080000),
    RIN_NET_PROTOCOL_DEFAULT = UINT32_C(0),
    RIN_NET_PROTOCOL_TCP = UINT32_C(6),
    RIN_NET_PROTOCOL_UDP = UINT32_C(17),
    RIN_NET_PROTOCOL_ICMPV6 = UINT32_C(58),
};

enum {
    RIN_NET_SOCKET_IO_PEEK = UINT32_C(0x00000001),
    RIN_NET_SOCKET_IO_WAIT_ALL = UINT32_C(0x00000002),
    RIN_NET_SOCKET_IO_DONT_WAIT = UINT32_C(0x00000004),
    RIN_NET_SOCKET_IO_FLAGS_ALL = RIN_NET_SOCKET_IO_PEEK |
                                  RIN_NET_SOCKET_IO_WAIT_ALL |
                                  RIN_NET_SOCKET_IO_DONT_WAIT
};

#define RIN_NET_TIMEOUT_INFINITE UINT64_MAX

/* rin_socket_shutdown_v1() takes a shutdown mode (matching POSIX SHUT_*),
 * not a bitmask. */
enum {
    RIN_NET_SOCKET_SHUTDOWN_RECEIVE = UINT32_C(0),
    RIN_NET_SOCKET_SHUTDOWN_SEND = UINT32_C(1),
    RIN_NET_SOCKET_SHUTDOWN_BOTH = UINT32_C(2)
};

/* TLS shutdown sends close_notify. Version 1 has no shutdown options. */
#define RIN_NET_TLS_SHUTDOWN_FLAGS_NONE UINT32_C(0)

/* TCP listen() accepts zero as the default backlog and caps requests above
 * the system maximum. POSIX callers pass a signed value and reject negatives. */
#define RIN_NET_TCP_LISTEN_BACKLOG_DEFAULT UINT32_C(128)
#define RIN_NET_TCP_LISTEN_BACKLOG_MAX UINT32_C(4096)

/* HTTP v1 uses bounded HTTP/1.1 messages. `headers` is a byte string of
 * CRLF-separated field lines without a final blank line. The returned
 * response channel contains the raw response bytes, including any interim
 * 1xx response blocks preceding the final response. `timeout_ns` must be
 * nonzero; RIN_NET_TIMEOUT_INFINITE disables the deadline. */
#define RIN_HTTP_REQUEST_HEADERS_MAX_BYTES UINT64_C(16384)
#define RIN_HTTP_RESPONSE_MAX_BYTES UINT64_C(65536)

typedef struct RinHttpRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    RinStringV1 method;
    RinStringV1 url;
    RinSliceV1 headers;
    RinSliceV1 body;
    uint64_t timeout_ns;
    uint64_t flags;
    uint64_t reserved[3];
} RinHttpRequestV1;

typedef struct RinSocketAcceptV1 {
    uint32_t struct_size;
    uint32_t version;
    RinSocket socket;
    RinSocketAddressV1 peer;
    uint64_t reserved[3];
} RinSocketAcceptV1;

RIN_SDK_API RinResult rin_socket_create_v1(uint32_t domain, uint32_t type, uint32_t protocol, RinSocket* socket);
RIN_SDK_API RinResult rin_socket_connect_v1(RinSocket socket, const RinSocketAddressV1* address);
RIN_SDK_API RinResult rin_socket_send_v1(RinSocket socket, RinSliceV1 data, uint32_t flags, uint64_t* sent);
RIN_SDK_API RinResult rin_socket_receive_v1(RinSocket socket, RinSliceV1 data, uint32_t flags, uint64_t* received);
/* These operations support datagram sockets. A non-NULL address must be a
 * version-1 RinSocketAddressV1. A NULL destination uses the connected peer;
 * a NULL source_address discards the sender address. Zero-length datagrams are
 * valid. receive accepts PEEK and DONT_WAIT (WAIT_ALL and truncation flags are
 * invalid); an oversized datagram is consumed in full while the byte count
 * reports only copied data. PEEK leaves it queued. receive commits only after
 * payload, optional source address, and optional byte count have copied. */
RIN_SDK_API RinResult rin_socket_sendto_v1(RinSocket socket, RinSliceV1 data, const RinSocketAddressV1* destination, uint32_t flags, uint64_t* sent);
RIN_SDK_API RinResult rin_socket_recvfrom_v1(RinSocket socket, RinSliceV1 data, uint32_t flags, RinSocketAddressV1* source_address, uint64_t* received);
RIN_SDK_API RinResult rin_dns_resolve_v1(RinStringV1 host, RinStringV1 service, RinSliceV1 results, uint64_t* count);
RIN_SDK_API RinResult rin_tls_connect_v1(RinSocket socket, RinStringV1 server_name, uint32_t flags, RinTlsSession* session);
RIN_SDK_API RinResult rin_http_execute_v1(const RinHttpRequestV1* request, RinChannel* response_stream);
RIN_SDK_API RinResult rin_tls_send_v1(RinTlsSession session, RinSliceV1 data, uint32_t flags, uint64_t* sent);
RIN_SDK_API RinResult rin_tls_receive_v1(RinTlsSession session, RinSliceV1 data, uint32_t flags, uint64_t* received);
RIN_SDK_API RinResult rin_tls_shutdown_v1(RinTlsSession session, uint32_t flags);
RIN_SDK_API RinResult rin_socket_bind_v1(RinSocket socket, const RinSocketAddressV1* address);
RIN_SDK_API RinResult rin_socket_listen_v1(RinSocket socket, uint32_t backlog);
/* timeout_ns == 0 performs an immediate accept attempt; INFINITE waits without
 * a deadline. */
RIN_SDK_API RinResult rin_socket_accept_v1(RinSocket socket, uint64_t timeout_ns, RinSocketAcceptV1* accepted);
RIN_SDK_API RinResult rin_socket_shutdown_v1(RinSocket socket, uint32_t how);

#ifdef __cplusplus
}
#endif
#endif
