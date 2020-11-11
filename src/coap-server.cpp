#include "coap-server.hpp"

#include <coap2/coap.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <netdb.h>
#include <atomic>
#include <cstdio>

extern std::atomic_ushort load_progress;

int first_ipv4_address(char chosen_address[])
{
    struct ifaddrs *available_addresses;
    if (getifaddrs(&available_addresses) == -1) {
        return -1;
    }
    struct ifaddrs *address_iter = available_addresses;
    while (address_iter) {
        int address_family = address_iter->ifa_addr->sa_family;
        if (address_family == AF_INET || address_family == AF_INET6) {
            // Address is either IPv4 address
            char user_friendly_address[100];
            const int address_info_size = (address_family == AF_INET)?
                                          sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            getnameinfo(address_iter->ifa_addr, address_info_size,
                        user_friendly_address, sizeof(user_friendly_address),
                        0, 0, NI_NUMERICHOST);
            if (strcmp(user_friendly_address, "127.0.0.1") != 0) {
                memcpy(chosen_address, user_friendly_address, strlen(user_friendly_address) + 1);
                return 0;
            }
        }
        address_iter = address_iter->ifa_next;
    }
    freeifaddrs(available_addresses);
    return -1;
}

int resolve_address(const char * host, const char * service, coap_address_t * dst)
{
    struct addrinfo *res, *ainfo;
    struct addrinfo hints;
    int len=-1;

    memset(&hints, 0, sizeof(hints));
    memset(dst, 0, sizeof(*dst));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    if (getaddrinfo(host, service, &hints, &res) != 0) {
        throw 404;
    }

    for (ainfo = res; ainfo != nullptr; ainfo = ainfo->ai_next) {
        switch (ainfo->ai_family) {
            case AF_INET6:
            case AF_INET:
                len = dst->size = ainfo->ai_addrlen;
                memcpy(&dst->addr.sin6, ainfo->ai_addr, dst->size);
                freeaddrinfo(res);
                return len;
            default:
                ;
        }
    }
    freeaddrinfo(res);
    return len;
}

void start_coap_server() {
    coap_context_t *ctx = nullptr;
    coap_address_t dst;
    coap_resource_t *resource = nullptr;
    coap_endpoint_t *endpoint = nullptr;

    coap_str_const_t *ruri = coap_make_str_const("hello");
    coap_startup();

    try {
        // Resolve destination address where the server will operate
        char hostname[100];
        if (first_ipv4_address(hostname) < 0) {
            throw 403;
        }

        if (resolve_address(hostname, "5683", &dst) < 0) {
            throw 404;
        }

    } catch (int errcode) {
        std::cout << "Could not resolve COAP server address, " << errcode << std::endl;
        return;
    }

    ctx = coap_new_context(nullptr);

    if (!ctx || !(endpoint = coap_new_endpoint(ctx, &dst, COAP_PROTO_UDP))) {
        std::cout << "Could not initialize context" << std::endl;
        return;
    }

    resource = coap_resource_init(ruri, 0);
    coap_register_handler(resource, COAP_REQUEST_GET,
                          [](auto, auto, auto, auto, auto, auto, coap_pdu_t *response)
                          {
                              std::cout << "Get request from someone" << std::endl;
                              response->code = COAP_RESPONSE_CODE(205);
                              coap_add_data(response, 5, (const uint8_t *)"world");
                          });

    coap_add_resource(ctx, resource);

    std::cout << "CoAP server is on port 5683\n" << std::flush;
    ++load_progress;

    while (true) { coap_run_once (ctx, 0); }

    coap_free_context(ctx);
    coap_cleanup();
    std::cout << "Exiting CoAP server" << std::endl;
    return;
}
