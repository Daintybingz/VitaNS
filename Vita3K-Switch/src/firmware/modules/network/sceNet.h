#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>

namespace firmware {
namespace modules {
namespace network {

// Network error codes
enum SceNetErrorCode {
    SCE_NET_ERROR_EPERM = 0x80410101,
    SCE_NET_ERROR_ENOENT = 0x80410102,
    SCE_NET_ERROR_ESRCH = 0x80410103,
    SCE_NET_ERROR_EINTR = 0x80410104,
    SCE_NET_ERROR_EIO = 0x80410105,
    SCE_NET_ERROR_ENXIO = 0x80410106,
    SCE_NET_ERROR_E2BIG = 0x80410107,
    SCE_NET_ERROR_ENOEXEC = 0x80410108,
    SCE_NET_ERROR_EBADF = 0x80410109,
    SCE_NET_ERROR_ECHILD = 0x8041010A,
    SCE_NET_ERROR_EAGAIN = 0x8041010B,
    SCE_NET_ERROR_ENOMEM = 0x8041010C,
    SCE_NET_ERROR_EACCES = 0x8041010D,
    SCE_NET_ERROR_EFAULT = 0x8041010E,
    SCE_NET_ERROR_ENOTBLK = 0x8041010F,
    SCE_NET_ERROR_EBUSY = 0x80410110,
    SCE_NET_ERROR_EEXIST = 0x80410111,
    SCE_NET_ERROR_EXDEV = 0x80410112,
    SCE_NET_ERROR_ENODEV = 0x80410113,
    SCE_NET_ERROR_ENOTDIR = 0x80410114,
    SCE_NET_ERROR_EISDIR = 0x80410115,
    SCE_NET_ERROR_EINVAL = 0x80410116,
    SCE_NET_ERROR_ENFILE = 0x80410117,
    SCE_NET_ERROR_EMFILE = 0x80410118,
    SCE_NET_ERROR_ENOTTY = 0x80410119,
    SCE_NET_ERROR_ETXTBSY = 0x8041011A,
    SCE_NET_ERROR_EFBIG = 0x8041011B,
    SCE_NET_ERROR_ENOSPC = 0x8041011C,
    SCE_NET_ERROR_ESPIPE = 0x8041011D,
    SCE_NET_ERROR_EROFS = 0x8041011E,
    SCE_NET_ERROR_EMLINK = 0x8041011F,
    SCE_NET_ERROR_EPIPE = 0x80410120,
    SCE_NET_ERROR_EDOM = 0x80410121,
    SCE_NET_ERROR_ERANGE = 0x80410122,
    SCE_NET_ERROR_ENOMSG = 0x80410123,
    SCE_NET_ERROR_EIDRM = 0x80410124,
    SCE_NET_ERROR_ECHRNG = 0x80410125,
    SCE_NET_ERROR_EL2NSYNC = 0x80410126,
    SCE_NET_ERROR_EL3HLT = 0x80410127,
    SCE_NET_ERROR_EL3RST = 0x80410128,
    SCE_NET_ERROR_ELNRNG = 0x80410129,
    SCE_NET_ERROR_EUNATCH = 0x8041012A,
    SCE_NET_ERROR_ENOCSI = 0x8041012B,
    SCE_NET_ERROR_EL2HLT = 0x8041012C,
    SCE_NET_ERROR_EDEADLK = 0x8041012D,
    SCE_NET_ERROR_ENOLCK = 0x8041012E,
    SCE_NET_ERROR_EFORMAT = 0x8041012F,
    SCE_NET_ERROR_EUNSUP = 0x80410130,
    SCE_NET_ERROR_EBADE = 0x80410131,
    SCE_NET_ERROR_EBADR = 0x80410132,
    SCE_NET_ERROR_EXFULL = 0x80410133,
    SCE_NET_ERROR_ENOANO = 0x80410134,
    SCE_NET_ERROR_EBADRQC = 0x80410135,
    SCE_NET_ERROR_EBADSLT = 0x80410136,
    SCE_NET_ERROR_EDEADLOCK = 0x80410137,
    SCE_NET_ERROR_EBFONT = 0x80410138,
    SCE_NET_ERROR_ENOSTR = 0x80410139,
    SCE_NET_ERROR_ENODATA = 0x8041013A,
    SCE_NET_ERROR_ETIME = 0x8041013B,
    SCE_NET_ERROR_ENOSR = 0x8041013C,
    SCE_NET_ERROR_ENONET = 0x8041013D,
    SCE_NET_ERROR_ENOPKG = 0x8041013E,
    SCE_NET_ERROR_EREMOTE = 0x8041013F,
    SCE_NET_ERROR_ENOLINK = 0x80410140,
    SCE_NET_ERROR_EADV = 0x80410141,
    SCE_NET_ERROR_ESRMNT = 0x80410142,
    SCE_NET_ERROR_ECOMM = 0x80410143,
    SCE_NET_ERROR_EPROTO = 0x80410144,
    SCE_NET_ERROR_EMULTIHOP = 0x80410145,
    SCE_NET_ERROR_EBADMSG = 0x80410146,
    SCE_NET_ERROR_EOVERFLOW = 0x80410147,
    SCE_NET_ERROR_ENOTUNIQ = 0x80410148,
    SCE_NET_ERROR_EBADFD = 0x80410149,
    SCE_NET_ERROR_EREMCHG = 0x8041014A,
    SCE_NET_ERROR_ELIBACC = 0x8041014B,
    SCE_NET_ERROR_ELIBBAD = 0x8041014C,
    SCE_NET_ERROR_ELIBSCN = 0x8041014D,
    SCE_NET_ERROR_ELIBMAX = 0x8041014E,
    SCE_NET_ERROR_ELIBEXEC = 0x8041014F,
    SCE_NET_ERROR_EILSEQ = 0x80410150,
    SCE_NET_ERROR_ENOSYS = 0x80410151,
    SCE_NET_ERROR_ELOOP = 0x80410152,
    SCE_NET_ERROR_ERESTART = 0x80410153,
    SCE_NET_ERROR_ESTRPIPE = 0x80410154,
    SCE_NET_ERROR_ENOTEMPTY = 0x80410155,
    SCE_NET_ERROR_ENOTINIT = 0x80410156,
    SCE_NET_ERROR_EUSERS = 0x80410157,
    SCE_NET_ERROR_ENOTSOCK = 0x80410158,
    SCE_NET_ERROR_EDESTADDRREQ = 0x80410159,
    SCE_NET_ERROR_EMSGSIZE = 0x8041015A,
    SCE_NET_ERROR_EPROTOTYPE = 0x8041015B,
    SCE_NET_ERROR_ENOPROTOOPT = 0x8041015C,
    SCE_NET_ERROR_EPROTONOSUPPORT = 0x8041015D,
    SCE_NET_ERROR_EOPNOTSUPP = 0x8041015E,
    SCE_NET_ERROR_EAFNOSUPPORT = 0x8041015F,
    SCE_NET_ERROR_EADDRINUSE = 0x80410160,
    SCE_NET_ERROR_EADDRNOTAVAIL = 0x80410161,
    SCE_NET_ERROR_ENETDOWN = 0x80410162,
    SCE_NET_ERROR_ENETUNREACH = 0x80410163,
    SCE_NET_ERROR_ENETRESET = 0x80410164,
    SCE_NET_ERROR_ECONNABORTED = 0x80410165,
    SCE_NET_ERROR_ECONNRESET = 0x80410166,
    SCE_NET_ERROR_ENOBUFS = 0x80410167,
    SCE_NET_ERROR_EISCONN = 0x80410168,
    SCE_NET_ERROR_ENOTCONN = 0x80410169,
    SCE_NET_ERROR_ETIMEDOUT = 0x8041016A,
    SCE_NET_ERROR_ECONNREFUSED = 0x8041016B,
    SCE_NET_ERROR_EHOSTUNREACH = 0x8041016C,
    SCE_NET_ERROR_EALREADY = 0x8041016D,
    SCE_NET_ERROR_EINPROGRESS = 0x8041016E,
    SCE_NET_ERROR_ESTALE = 0x8041016F,
    SCE_NET_ERROR_EAUTH = 0x80410170,
    SCE_NET_ERROR_ENEEDAUTH = 0x80410171,
    SCE_NET_ERROR_ECANCELED = 0x80410172,
    SCE_NET_ERROR_EINTERNAL = 0x80410173,
    SCE_NET_ERROR_EBADF_INTERNAL = 0x80410174,
    SCE_NET_ERROR_EMAXRETRY = 0x80410175,
    SCE_NET_ERROR_ENOTSUP = 0x80410176,
    SCE_NET_ERROR_EPROCLIM = 0x80410177,
    SCE_NET_ERROR_EUSERS_INTERNAL = 0x80410178,
    SCE_NET_ERROR_ENOTREADY = 0x80410179,
    SCE_NET_ERROR_EOPFAILED = 0x8041017A,
    SCE_NET_ERROR_EBIGLIBRARY = 0x8041017B,
    SCE_NET_ERROR_EFDOPEN = 0x8041017C,
    SCE_NET_ERROR_EGARBLED = 0x8041017D
};

// Network protocol definitions
#define SCE_NET_AF_INET 2
#define SCE_NET_AF_INET6 24

#define SCE_NET_SOCK_STREAM 1
#define SCE_NET_SOCK_DGRAM 2
#define SCE_NET_SOCK_RAW 3

#define SCE_NET_IPPROTO_IP 0
#define SCE_NET_IPPROTO_ICMP 1
#define SCE_NET_IPPROTO_IGMP 2
#define SCE_NET_IPPROTO_TCP 6
#define SCE_NET_IPPROTO_UDP 17
#define SCE_NET_IPPROTO_ICMPV6 58

// Socket option definitions
#define SCE_NET_SOL_SOCKET 0xFFFF
#define SCE_NET_SO_REUSEADDR 0x00000004
#define SCE_NET_SO_KEEPALIVE 0x00000008
#define SCE_NET_SO_BROADCAST 0x00000020
#define SCE_NET_SO_LINGER 0x00000080
#define SCE_NET_SO_OOBINLINE 0x00000100
#define SCE_NET_SO_REUSEPORT 0x00000200
#define SCE_NET_SO_ONESBCAST 0x00000800
#define SCE_NET_SO_USECRYPTO 0x00001000
#define SCE_NET_SO_USESIGNATURE 0x00002000
#define SCE_NET_SO_SNDBUF 0x1001
#define SCE_NET_SO_RCVBUF 0x1002
#define SCE_NET_SO_SNDLOWAT 0x1003
#define SCE_NET_SO_RCVLOWAT 0x1004
#define SCE_NET_SO_SNDTIMEO 0x1005
#define SCE_NET_SO_RCVTIMEO 0x1006
#define SCE_NET_SO_ERROR 0x1007
#define SCE_NET_SO_TYPE 0x1008
#define SCE_NET_SO_NBIO 0x1100
#define SCE_NET_SO_TPPOLICY 0x1101

// Network initialization parameters
struct SceNetInitParam {
    uint32_t memory_size;
    uint32_t memory_pool;
};

// Socket address structure
struct SceNetSockaddr {
    uint8_t sa_len;
    uint8_t sa_family;
    char sa_data[14];
};

// Internet address structure
struct SceNetInAddr {
    uint32_t s_addr;
};

// Internet socket address structure
struct SceNetSockaddrIn {
    uint8_t sin_len;
    uint8_t sin_family;
    uint16_t sin_port;
    SceNetInAddr sin_addr;
    uint8_t sin_zero[8];
};

// IPv6 address structure
struct SceNetIn6Addr {
    uint8_t s6_addr[16];
};

// IPv6 socket address structure
struct SceNetSockaddrIn6 {
    uint8_t sin6_len;
    uint8_t sin6_family;
    uint16_t sin6_port;
    uint32_t sin6_flowinfo;
    SceNetIn6Addr sin6_addr;
    uint32_t sin6_scope_id;
};

// Host entry structure
struct SceNetHostent {
    char *h_name;
    char **h_aliases;
    int h_addrtype;
    int h_length;
    char **h_addr_list;
};

// Poll descriptor
struct SceNetPollfd {
    int fd;
    int events;
    int revents;
};

// Network statistics
struct SceNetStatisticsInfo {
    int kernel_mem_size;
    int kernel_mem_used;
    int packet_count;
    int packet_drop_count;
};

// Network interface information
struct SceNetIfInfo {
    char name[16];
    uint32_t flags;
    SceNetInAddr addr;
    SceNetInAddr mask;
    SceNetInAddr broadcast;
    uint8_t hwaddr[8];
    uint32_t mtu;
    uint32_t link;
    uint32_t media;
    uint32_t speed;
    uint32_t duplex;
    uint32_t inactive;
    uint32_t reserved[8];
};

// Forward declaration for addrinfo and ifaddrs
struct addrinfo;
struct ifaddrs;

// Network manager class
class SceNetManager {
public:
    SceNetManager();
    ~SceNetManager();

    // Initialize the network manager
    int initialize(const SceNetInitParam *param);

    // Terminate the network manager
    int terminate();

    // Socket API
    int socket(int domain, int type, int protocol);
    int close(int s);
    int connect(int s, const SceNetSockaddr *name, uint32_t namelen);
    int bind(int s, const SceNetSockaddr *name, uint32_t namelen);
    int listen(int s, int backlog);
    int accept(int s, SceNetSockaddr *addr, uint32_t *addrlen);
    int send(int s, const void *msg, uint32_t len, int flags);
    int recv(int s, void *buf, uint32_t len, int flags);
    int sendto(int s, const void *msg, uint32_t len, int flags, const SceNetSockaddr *to, uint32_t tolen);
    int recvfrom(int s, void *buf, uint32_t len, int flags, SceNetSockaddr *from, uint32_t *fromlen);
    int setsockopt(int s, int level, int optname, const void *optval, uint32_t optlen);
    int getsockopt(int s, int level, int optname, void *optval, uint32_t *optlen);
    int getsockname(int s, SceNetSockaddr *name, uint32_t *namelen);
    int getpeername(int s, SceNetSockaddr *name, uint32_t *namelen);
    int shutdown(int s, int how);
    int poll(SceNetPollfd *fds, uint32_t nfds, int timeout);
    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

    // DNS API
    SceNetHostent *gethostbyname(const char *name);
    SceNetHostent *gethostbyaddr(const void *addr, uint32_t len, int type);
    int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
    void freeaddrinfo(struct addrinfo *ai);
    int getnameinfo(const SceNetSockaddr *sa, uint32_t salen, char *host, uint32_t hostlen, char *serv, uint32_t servlen, int flags);

    // Interface API
    int getifaddrs(struct ifaddrs **ifap);
    void freeifaddrs(struct ifaddrs *ifa);
    int getsockstat(int s, int code, void *buf, uint32_t len);
    int getifstat(const char *name, int code, void *buf, uint32_t len);

    // Utility API
    uint32_t inet_addr(const char *cp);
    char *inet_ntoa(SceNetInAddr in);
    int inet_pton(int af, const char *src, void *dst);
    const char *inet_ntop(int af, const void *src, char *dst, uint32_t size);

    // Statistics API
    int get_statistics(SceNetStatisticsInfo *info);

private:
    // Network memory
    void *network_memory;
    uint32_t network_memory_size;

    // Socket map
    std::map<int, int> socket_map; // PS Vita socket ID -> Host socket ID

    // Host entry cache
    std::map<std::string, SceNetHostent*> hostent_cache;

    // Network initialized flag
    bool initialized;

    // Mutex for thread safety
    std::mutex mutex;

    // Convert PS Vita socket address to host socket address
    bool convert_sockaddr_to_host(const SceNetSockaddr *vita_addr, uint32_t vita_addrlen, struct sockaddr *host_addr, socklen_t *host_addrlen);

    // Convert host socket address to PS Vita socket address
    bool convert_sockaddr_to_vita(const struct sockaddr *host_addr, socklen_t host_addrlen, SceNetSockaddr *vita_addr, uint32_t *vita_addrlen);

    // Allocate a new socket ID
    int allocate_socket_id(int host_socket);

    // Free a socket ID
    void free_socket_id(int vita_socket);

    // Get host socket from PS Vita socket
    int get_host_socket(int vita_socket);
};

// Module functions
int sceNetInit(const SceNetInitParam *param);
int sceNetTerm();
int sceNetShowIfconfig();
int sceNetShowRoute();
int sceNetShowNetstat();
int sceNetSocket(const char *name, int domain, int type, int protocol);
int sceNetAccept(int s, SceNetSockaddr *addr, uint32_t *addrlen);
int sceNetBind(int s, const SceNetSockaddr *addr, uint32_t addrlen);
int sceNetConnect(int s, const SceNetSockaddr *name, uint32_t namelen);
int sceNetListen(int s, int backlog);
int sceNetRecv(int s, void *buf, uint32_t len, int flags);
int sceNetRecvfrom(int s, void *buf, uint32_t len, int flags, SceNetSockaddr *from, uint32_t *fromlen);
int sceNetSend(int s, const void *msg, uint32_t len, int flags);
int sceNetSendto(int s, const void *msg, uint32_t len, int flags, const SceNetSockaddr *to, uint32_t tolen);
int sceNetSetsockopt(int s, int level, int optname, const void *optval, uint32_t optlen);
int sceNetGetsockopt(int s, int level, int optname, void *optval, uint32_t *optlen);
int sceNetShutdown(int s, int how);
int sceNetClose(int s);
int sceNetGetsockname(int s, SceNetSockaddr *name, uint32_t *namelen);
int sceNetGetpeername(int s, SceNetSockaddr *name, uint32_t *namelen);
int sceNetInetPton(int af, const char *src, void *dst);
const char *sceNetInetNtop(int af, const void *src, char *dst, uint32_t size);
uint32_t sceNetHtonl(uint32_t host32);
uint16_t sceNetHtons(uint16_t host16);
uint32_t sceNetNtohl(uint32_t net32);
uint16_t sceNetNtohs(uint16_t net16);
int sceNetGetstatByName(const char *name, int stat, void *buf, uint32_t len);
int sceNetGetstat(int s, int stat, void *buf, uint32_t len);

} // namespace network
} // namespace modules
} // namespace firmware
