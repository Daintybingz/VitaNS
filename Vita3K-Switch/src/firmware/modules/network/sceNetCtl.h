#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <functional>

namespace firmware {
namespace modules {
namespace network {

// Network control error codes
enum SceNetCtlErrorCode {
    SCE_NETCTL_ERROR_NOT_INITIALIZED      = 0x80412101,
    SCE_NETCTL_ERROR_NOT_TERMINATED       = 0x80412102,
    SCE_NETCTL_ERROR_CALLBACK_NOT_FOUND   = 0x80412103,
    SCE_NETCTL_ERROR_CALLBACK_MAX         = 0x80412104,
    SCE_NETCTL_ERROR_INVALID_ID           = 0x80412105,
    SCE_NETCTL_ERROR_INVALID_ADDR         = 0x80412106,
    SCE_NETCTL_ERROR_NOT_CONNECTED        = 0x80412107,
    SCE_NETCTL_ERROR_NOT_AVAIL            = 0x80412108,
    SCE_NETCTL_ERROR_INVALID_TYPE         = 0x80412109,
    SCE_NETCTL_ERROR_INVALID_SIZE         = 0x8041210a,
    SCE_NETCTL_ERROR_NET_INTERNAL         = 0x8041210b,
    SCE_NETCTL_ERROR_WIFI_INVALID_HANDLER = 0x8041210c,
    SCE_NETCTL_ERROR_WIFI_INVALID_SSID    = 0x8041210d,
    SCE_NETCTL_ERROR_WIFI_INVALID_PARAM   = 0x8041210e,
    SCE_NETCTL_ERROR_WIFI_NO_PARAM        = 0x8041210f,
    SCE_NETCTL_ERROR_WIFI_INVALID_SECURITY= 0x80412110,
    SCE_NETCTL_ERROR_WIFI_CONNECT_TIMEOUT = 0x80412111,
    SCE_NETCTL_ERROR_WIFI_ALREADY_CONNECTED = 0x80412112,
    SCE_NETCTL_ERROR_WIFI_DISCONNECTED    = 0x80412113,
    SCE_NETCTL_ERROR_WIFI_NOT_FOUND       = 0x80412114,
    SCE_NETCTL_ERROR_WIFI_INVALID_EAPOL   = 0x80412115,
    SCE_NETCTL_ERROR_WIFI_BUSY            = 0x80412116,
    SCE_NETCTL_ERROR_WIFI_FATAL           = 0x80412117,
    SCE_NETCTL_ERROR_WIFI_SCAN_TIMEOUT    = 0x80412118,
    SCE_NETCTL_ERROR_WIFI_SCAN_BUSY       = 0x80412119,
    SCE_NETCTL_ERROR_WIFI_INVALID_SCANINFO= 0x8041211a,
    SCE_NETCTL_ERROR_WIFI_INVALID_MODE    = 0x8041211b
};

// Network connection state
enum SceNetCtlState {
    SCE_NETCTL_STATE_DISCONNECTED         = 0,
    SCE_NETCTL_STATE_CONNECTING           = 1,
    SCE_NETCTL_STATE_CONNECTED            = 2
};

// Network connection type
enum SceNetCtlType {
    SCE_NETCTL_INFO_CONFIG_NAME           = 1,
    SCE_NETCTL_INFO_DEVICE                = 2,
    SCE_NETCTL_INFO_ETHER_ADDR            = 3,
    SCE_NETCTL_INFO_MTU                   = 4,
    SCE_NETCTL_INFO_LINK                  = 5,
    SCE_NETCTL_INFO_BSSID                 = 6,
    SCE_NETCTL_INFO_SSID                  = 7,
    SCE_NETCTL_INFO_WIFI_SECURITY         = 8,
    SCE_NETCTL_INFO_RSSI_DBM              = 9,
    SCE_NETCTL_INFO_RSSI_PERCENTAGE       = 10,
    SCE_NETCTL_INFO_CHANNEL               = 11,
    SCE_NETCTL_INFO_IP_CONFIG             = 12,
    SCE_NETCTL_INFO_DHCP_HOSTNAME         = 13,
    SCE_NETCTL_INFO_PPPOE_AUTH_NAME       = 14,
    SCE_NETCTL_INFO_IP_ADDRESS            = 15,
    SCE_NETCTL_INFO_NETMASK               = 16,
    SCE_NETCTL_INFO_DEFAULT_ROUTE         = 17,
    SCE_NETCTL_INFO_PRIMARY_DNS           = 18,
    SCE_NETCTL_INFO_SECONDARY_DNS         = 19,
    SCE_NETCTL_INFO_HTTP_PROXY_CONFIG     = 20,
    SCE_NETCTL_INFO_HTTP_PROXY_SERVER     = 21,
    SCE_NETCTL_INFO_HTTP_PROXY_PORT       = 22
};

// Network interface type
enum SceNetCtlIfType {
    SCE_NETCTL_IFTYPE_NONE                = 0,
    SCE_NETCTL_IFTYPE_WIRED               = 1,
    SCE_NETCTL_IFTYPE_WIFI                = 2
};

// Network information structure
struct SceNetCtlInfo {
    union {
        char name[64];                    // SCE_NETCTL_INFO_CONFIG_NAME
        uint32_t device;                  // SCE_NETCTL_INFO_DEVICE
        char ether_addr[6];               // SCE_NETCTL_INFO_ETHER_ADDR
        uint32_t mtu;                     // SCE_NETCTL_INFO_MTU
        uint32_t link;                    // SCE_NETCTL_INFO_LINK
        char bssid[6];                    // SCE_NETCTL_INFO_BSSID
        char ssid[32];                    // SCE_NETCTL_INFO_SSID
        uint32_t wifi_security;           // SCE_NETCTL_INFO_WIFI_SECURITY
        uint8_t rssi_dbm;                 // SCE_NETCTL_INFO_RSSI_DBM
        uint8_t rssi_percentage;          // SCE_NETCTL_INFO_RSSI_PERCENTAGE
        uint8_t channel;                  // SCE_NETCTL_INFO_CHANNEL
        uint32_t ip_config;               // SCE_NETCTL_INFO_IP_CONFIG
        char dhcp_hostname[256];          // SCE_NETCTL_INFO_DHCP_HOSTNAME
        char pppoe_auth_name[128];        // SCE_NETCTL_INFO_PPPOE_AUTH_NAME
        char ip_address[16];              // SCE_NETCTL_INFO_IP_ADDRESS
        char netmask[16];                 // SCE_NETCTL_INFO_NETMASK
        char default_route[16];           // SCE_NETCTL_INFO_DEFAULT_ROUTE
        char primary_dns[16];             // SCE_NETCTL_INFO_PRIMARY_DNS
        char secondary_dns[16];           // SCE_NETCTL_INFO_SECONDARY_DNS
        uint32_t http_proxy_config;       // SCE_NETCTL_INFO_HTTP_PROXY_CONFIG
        char http_proxy_server[256];      // SCE_NETCTL_INFO_HTTP_PROXY_SERVER
        uint16_t http_proxy_port;         // SCE_NETCTL_INFO_HTTP_PROXY_PORT
    };
};

// Network event type
enum SceNetCtlEvent {
    SCE_NETCTL_EVENT_CONNECT_REQUEST      = 0,
    SCE_NETCTL_EVENT_ESTABLISH            = 1,
    SCE_NETCTL_EVENT_GET_IP               = 2,
    SCE_NETCTL_EVENT_DISCONNECT_REQUEST   = 3,
    SCE_NETCTL_EVENT_ERROR                = 4,
    SCE_NETCTL_EVENT_LINK_UP              = 5,
    SCE_NETCTL_EVENT_LINK_DOWN            = 6,
    SCE_NETCTL_EVENT_SCAN_COMPLETE        = 7,
    SCE_NETCTL_EVENT_SCAN_STOP            = 8,
    SCE_NETCTL_EVENT_IPCFG_CHANGED        = 9
};

// Network callback function type
typedef void (*SceNetCtlCallback)(int event_type, void *arg);

// Network control manager class
class SceNetCtlManager {
public:
    SceNetCtlManager();
    ~SceNetCtlManager();

    // Initialize the network control manager
    int initialize();

    // Terminate the network control manager
    int terminate();

    // Get network state
    int get_state(int *state);

    // Get network information
    int get_info(int code, SceNetCtlInfo *info);

    // Register callback
    int register_callback(SceNetCtlCallback callback, void *arg, int *cid);

    // Unregister callback
    int unregister_callback(int cid);

    // Connect to network
    int connect();

    // Disconnect from network
    int disconnect();

    // Check if connected
    bool is_connected() const;

    // Trigger network event
    void trigger_event(int event_type);

private:
    // Network state
    int state;

    // Network information
    std::map<int, SceNetCtlInfo> info;

    // Callbacks
    struct CallbackEntry {
        SceNetCtlCallback callback;
        void *arg;
    };
    std::map<int, CallbackEntry> callbacks;

    // Next callback ID
    int next_callback_id;

    // Network initialized flag
    bool initialized;

    // Mutex for thread safety
    mutable std::mutex mutex;
};

// Module functions
int sceNetCtlInit();
int sceNetCtlTerm();
int sceNetCtlGetState(int *state);
int sceNetCtlGetInfo(int code, SceNetCtlInfo *info);
int sceNetCtlConnect();
int sceNetCtlDisconnect();
int sceNetCtlCheckCallback();
int sceNetCtlInetGetInfo(int code, SceNetCtlInfo *info);
int sceNetCtlInetGetState(int *state);
int sceNetCtlRegisterCallback(SceNetCtlCallback callback, void *arg, int *cid);
int sceNetCtlUnregisterCallback(int cid);

} // namespace network
} // namespace modules
} // namespace firmware
