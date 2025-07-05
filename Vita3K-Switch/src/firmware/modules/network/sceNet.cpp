#include "sceNet.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#if __has_include(<ifaddrs.h>)
#include <ifaddrs.h>
#endif
#include <net/if.h>
#include <fcntl.h>
#include <poll.h>
#endif

namespace firmware {
namespace modules {
namespace network {

// Global network manager instance
static SceNetManager g_net_manager;

// SceNetManager implementation
SceNetManager::SceNetManager()
    : network_memory(nullptr), network_memory_size(0), initialized(false) {
}

SceNetManager::~SceNetManager() {
    if (initialized) {
        terminate();
    }
}

int SceNetManager::initialize(const SceNetInitParam *param) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (initialized) {
        std::cerr << "Network manager is already initialized" << std::endl;
        return SCE_NET_ERROR_EINTERNAL;
    }
    
    if (!param) {
        std::cerr << "Invalid network initialization parameters" << std::endl;
        return SCE_NET_ERROR_EINVAL;
    }
    
    std::cout << "Initializing network manager with memory size " << param->memory_size << std::endl;
    
    // Initialize platform-specific network stack
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return SCE_NET_ERROR_EINTERNAL;
    }
#endif
    
    // Allocate network memory
    network_memory_size = param->memory_size;
    network_memory = malloc(network_memory_size);
    if (!network_memory) {
        std::cerr << "Failed to allocate network memory" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return SCE_NET_ERROR_ENOMEM;
    }
    
    // Clear memory
    memset(network_memory, 0, network_memory_size);
    
    // Initialize socket map and host entry cache
    socket_map.clear();
    hostent_cache.clear();
    
    initialized = true;
    std::cout << "Network manager initialized successfully" << std::endl;
    return 0;
}

int SceNetManager::terminate() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Network manager is not initialized" << std::endl;
        return SCE_NET_ERROR_ENOTINIT;
    }
    
    std::cout << "Terminating network manager" << std::endl;
    
    // Close all sockets
    for (auto &pair : socket_map) {
        int host_socket = pair.second;
#ifdef _WIN32
        closesocket(host_socket);
#else
        close(host_socket);
#endif
    }
    
    // Clear socket map
    socket_map.clear();
    
    // Free host entry cache
    for (auto &pair : hostent_cache) {
        SceNetHostent *hostent = pair.second;
        if (hostent) {
            if (hostent->h_name) {
                free(hostent->h_name);
            }
            if (hostent->h_aliases) {
                for (int i = 0; hostent->h_aliases[i]; i++) {
                    free(hostent->h_aliases[i]);
                }
                free(hostent->h_aliases);
            }
            if (hostent->h_addr_list) {
                for (int i = 0; hostent->h_addr_list[i]; i++) {
                    free(hostent->h_addr_list[i]);
                }
                free(hostent->h_addr_list);
            }
            free(hostent);
        }
    }
    
    // Clear host entry cache
    hostent_cache.clear();
    
    // Free network memory
    if (network_memory) {
        free(network_memory);
        network_memory = nullptr;
    }
    
    // Terminate platform-specific network stack
#ifdef _WIN32
    WSACleanup();
#endif
    
    initialized = false;
    std::cout << "Network manager terminated successfully" << std::endl;
    return 0;
}

int SceNetManager::socket(int domain, int type, int protocol) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Network manager is not initialized" << std::endl;
        return SCE_NET_ERROR_ENOTINIT;
    }
    
    // Convert PS Vita domain to host domain
    int host_domain;
    switch (domain) {
        case SCE_NET_AF_INET:
            host_domain = AF_INET;
            break;
        case SCE_NET_AF_INET6:
            host_domain = AF_INET6;
            break;
        default:
            std::cerr << "Unsupported socket domain: " << domain << std::endl;
            return SCE_NET_ERROR_EAFNOSUPPORT;
    }
    
    // Convert PS Vita type to host type
    int host_type;
    switch (type) {
        case SCE_NET_SOCK_STREAM:
            host_type = SOCK_STREAM;
            break;
        case SCE_NET_SOCK_DGRAM:
            host_type = SOCK_DGRAM;
            break;
        case SCE_NET_SOCK_RAW:
            host_type = SOCK_RAW;
            break;
        default:
            std::cerr << "Unsupported socket type: " << type << std::endl;
            return SCE_NET_ERROR_EPROTOTYPE;
    }
    
    // Convert PS Vita protocol to host protocol
    int host_protocol;
    switch (protocol) {
        case SCE_NET_IPPROTO_IP:
            host_protocol = IPPROTO_IP;
            break;
        case SCE_NET_IPPROTO_ICMP:
            host_protocol = IPPROTO_ICMP;
            break;
        case SCE_NET_IPPROTO_IGMP:
            host_protocol = IPPROTO_IGMP;
            break;
        case SCE_NET_IPPROTO_TCP:
            host_protocol = IPPROTO_TCP;
            break;
        case SCE_NET_IPPROTO_UDP:
            host_protocol = IPPROTO_UDP;
            break;
        case SCE_NET_IPPROTO_ICMPV6:
            host_protocol = IPPROTO_ICMPV6;
            break;
        default:
            std::cerr << "Unsupported socket protocol: " << protocol << std::endl;
            return SCE_NET_ERROR_EPROTONOSUPPORT;
    }
    
    // Create socket
    int host_socket = ::socket(host_domain, host_type, host_protocol);
    if (host_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return SCE_NET_ERROR_EINTERNAL;
    }
    
    // Allocate PS Vita socket ID
    int vita_socket = allocate_socket_id(host_socket);
    if (vita_socket < 0) {
        std::cerr << "Failed to allocate socket ID" << std::endl;
#ifdef _WIN32
        closesocket(host_socket);
#else
        close(host_socket);
#endif
        return SCE_NET_ERROR_EMFILE;
    }
    
    std::cout << "Created socket " << vita_socket << " (host socket: " << host_socket << ")" << std::endl;
    return vita_socket;
}

int SceNetManager::close(int s) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Network manager is not initialized" << std::endl;
        return SCE_NET_ERROR_ENOTINIT;
    }
    
    // Get host socket
    int host_socket = get_host_socket(s);
    if (host_socket < 0) {
        std::cerr << "Invalid socket: " << s << std::endl;
        return SCE_NET_ERROR_EBADF;
    }
    
    // Close host socket
#ifdef _WIN32
    if (closesocket(host_socket) != 0) {
        std::cerr << "Failed to close socket " << s << std::endl;
        return SCE_NET_ERROR_EINTERNAL;
    }
#else
    if (close(host_socket) != 0) {
        std::cerr << "Failed to close socket " << s << std::endl;
        return SCE_NET_ERROR_EINTERNAL;
    }
#endif
    
    // Free socket ID
    free_socket_id(s);
    
    std::cout << "Closed socket " << s << std::endl;
    return 0;
}

bool SceNetManager::convert_sockaddr_to_host(const SceNetSockaddr *vita_addr, uint32_t vita_addrlen, struct sockaddr *host_addr, socklen_t *host_addrlen) {
    if (!vita_addr || !host_addr || !host_addrlen) {
        return false;
    }
    
    // Convert based on address family
    switch (vita_addr->sa_family) {
        case SCE_NET_AF_INET: {
            if (vita_addrlen < sizeof(SceNetSockaddrIn)) {
                return false;
            }
            
            const SceNetSockaddrIn *vita_addr_in = reinterpret_cast<const SceNetSockaddrIn*>(vita_addr);
            struct sockaddr_in *host_addr_in = reinterpret_cast<struct sockaddr_in*>(host_addr);
            
            if (*host_addrlen < sizeof(struct sockaddr_in)) {
                return false;
            }
            
            memset(host_addr_in, 0, sizeof(struct sockaddr_in));
            host_addr_in->sin_family = AF_INET;
            host_addr_in->sin_port = vita_addr_in->sin_port;
            host_addr_in->sin_addr.s_addr = vita_addr_in->sin_addr.s_addr;
            
            *host_addrlen = sizeof(struct sockaddr_in);
            return true;
        }
        
        case SCE_NET_AF_INET6: {
            if (vita_addrlen < sizeof(SceNetSockaddrIn6)) {
                return false;
            }
            
            const SceNetSockaddrIn6 *vita_addr_in6 = reinterpret_cast<const SceNetSockaddrIn6*>(vita_addr);
            struct sockaddr_in6 *host_addr_in6 = reinterpret_cast<struct sockaddr_in6*>(host_addr);
            
            if (*host_addrlen < sizeof(struct sockaddr_in6)) {
                return false;
            }
            
            memset(host_addr_in6, 0, sizeof(struct sockaddr_in6));
            host_addr_in6->sin6_family = AF_INET6;
            host_addr_in6->sin6_port = vita_addr_in6->sin6_port;
            host_addr_in6->sin6_flowinfo = vita_addr_in6->sin6_flowinfo;
            memcpy(&host_addr_in6->sin6_addr, &vita_addr_in6->sin6_addr, sizeof(struct in6_addr));
            host_addr_in6->sin6_scope_id = vita_addr_in6->sin6_scope_id;
            
            *host_addrlen = sizeof(struct sockaddr_in6);
            return true;
        }
        
        default:
            std::cerr << "Unsupported address family: " << vita_addr->sa_family << std::endl;
            return false;
    }
}

bool SceNetManager::convert_sockaddr_to_vita(const struct sockaddr *host_addr, socklen_t host_addrlen, SceNetSockaddr *vita_addr, uint32_t *vita_addrlen) {
    if (!host_addr || !vita_addr || !vita_addrlen) {
        return false;
    }
    
    // Convert based on address family
    switch (host_addr->sa_family) {
        case AF_INET: {
            if (host_addrlen < sizeof(struct sockaddr_in)) {
                return false;
            }
            
            const struct sockaddr_in *host_addr_in = reinterpret_cast<const struct sockaddr_in*>(host_addr);
            SceNetSockaddrIn *vita_addr_in = reinterpret_cast<SceNetSockaddrIn*>(vita_addr);
            
            if (*vita_addrlen < sizeof(SceNetSockaddrIn)) {
                return false;
            }
            
            memset(vita_addr_in, 0, sizeof(SceNetSockaddrIn));
            vita_addr_in->sin_len = sizeof(SceNetSockaddrIn);
            vita_addr_in->sin_family = SCE_NET_AF_INET;
            vita_addr_in->sin_port = host_addr_in->sin_port;
            vita_addr_in->sin_addr.s_addr = host_addr_in->sin_addr.s_addr;
            
            *vita_addrlen = sizeof(SceNetSockaddrIn);
            return true;
        }
        
        case AF_INET6: {
            if (host_addrlen < sizeof(struct sockaddr_in6)) {
                return false;
            }
            
            const struct sockaddr_in6 *host_addr_in6 = reinterpret_cast<const struct sockaddr_in6*>(host_addr);
            SceNetSockaddrIn6 *vita_addr_in6 = reinterpret_cast<SceNetSockaddrIn6*>(vita_addr);
            
            if (*vita_addrlen < sizeof(SceNetSockaddrIn6)) {
                return false;
            }
            
            memset(vita_addr_in6, 0, sizeof(SceNetSockaddrIn6));
            vita_addr_in6->sin6_len = sizeof(SceNetSockaddrIn6);
            vita_addr_in6->sin6_family = SCE_NET_AF_INET6;
            vita_addr_in6->sin6_port = host_addr_in6->sin6_port;
            vita_addr_in6->sin6_flowinfo = host_addr_in6->sin6_flowinfo;
            memcpy(&vita_addr_in6->sin6_addr, &host_addr_in6->sin6_addr, sizeof(SceNetIn6Addr));
            vita_addr_in6->sin6_scope_id = host_addr_in6->sin6_scope_id;
            
            *vita_addrlen = sizeof(SceNetSockaddrIn6);
            return true;
        }
        
        default:
            std::cerr << "Unsupported address family: " << host_addr->sa_family << std::endl;
            return false;
    }
}

int SceNetManager::allocate_socket_id(int host_socket) {
    // Find the lowest available socket ID starting from 1
    int vita_socket = 1;
    while (socket_map.find(vita_socket) != socket_map.end()) {
        vita_socket++;
    }
    
    // Register the socket mapping
    socket_map[vita_socket] = host_socket;
    
    return vita_socket;
}

void SceNetManager::free_socket_id(int vita_socket) {
    socket_map.erase(vita_socket);
}

int SceNetManager::get_host_socket(int vita_socket) {
    auto it = socket_map.find(vita_socket);
    if (it == socket_map.end()) {
        return -1;
    }
    
    return it->second;
}

// Module function implementations
int sceNetInit(const SceNetInitParam *param) {
    return g_net_manager.initialize(param);
}

int sceNetTerm() {
    return g_net_manager.terminate();
}

int sceNetSocket(const char *name, int domain, int type, int protocol) {
    return g_net_manager.socket(domain, type, protocol);
}

int sceNetClose(int s) {
    return g_net_manager.close(s);
}

} // namespace network
} // namespace modules
} // namespace firmware
