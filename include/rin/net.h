#ifndef RIN_SDK_NET_H
#define RIN_SDK_NET_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct RinSocketAddressV1 {
    uint32_t struct_size;
    uint32_t version;
    uint16_t family;
    uint16_t port;
    uint32_t scope_id;
    uint8_t address[16];
    uint64_t reserved[2];
} RinSocketAddressV1;

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
RIN_SDK_API RinResult rin_dns_resolve_v1(RinStringV1 host, RinStringV1 service, RinSliceV1 results, uint64_t* count);
RIN_SDK_API RinResult rin_tls_connect_v1(RinSocket socket, RinStringV1 server_name, uint32_t flags, RinTlsSession* session);
RIN_SDK_API RinResult rin_http_execute_v1(const RinHttpRequestV1* request, RinChannel* response_stream);
RIN_SDK_API RinResult rin_tls_send_v1(RinTlsSession session, RinSliceV1 data, uint32_t flags, uint64_t* sent);
RIN_SDK_API RinResult rin_tls_receive_v1(RinTlsSession session, RinSliceV1 data, uint32_t flags, uint64_t* received);
RIN_SDK_API RinResult rin_tls_shutdown_v1(RinTlsSession session, uint32_t flags);
RIN_SDK_API RinResult rin_socket_bind_v1(RinSocket socket, const RinSocketAddressV1* address);
RIN_SDK_API RinResult rin_socket_listen_v1(RinSocket socket, uint32_t backlog);
RIN_SDK_API RinResult rin_socket_accept_v1(RinSocket socket, uint64_t timeout_ns, RinSocketAcceptV1* accepted);
RIN_SDK_API RinResult rin_socket_shutdown_v1(RinSocket socket, uint32_t flags);

#ifdef __cplusplus
}
#endif
#endif
