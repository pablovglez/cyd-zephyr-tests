//
// Created by efisio on 11/2/25.
//

#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/hostname.h>
#include "wifi_component.h"

static struct k_work_delayable wifi_reconnect_work;
static int reconnect_attempts;
static bool reconnecting;

LOG_MODULE_REGISTER(wifi);

int s_retry_num = 0;

bool device_online = 0;

char* global_wifi_ssid = NULL;
char* global_wifi_password = NULL;

struct wifi_connect_req_params wifi_params = {0};


static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

static K_SEM_DEFINE(wifi_connected, 0, 1);
static K_SEM_DEFINE(ipv4_address_obtained, 0, 1);

static void reconnect_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    if (device_online) {
        reconnecting = false;
        reconnect_attempts = 0;
        return;
    }

    // attempt to start a connection (connect_wifi is async)
    connect_wifi();
    reconnect_attempts++;

    if (reconnect_attempts < CONFIG_WIFI_MAX_RETRY_COUNT) {
        // quick retry loop
        k_work_reschedule(&wifi_reconnect_work, K_SECONDS(CONFIG_WIFI_SHORT_RETRY_DELAY_SECONDS));
    } else {
        // switch to longer periodic retry
        k_work_reschedule(&wifi_reconnect_work, K_SECONDS(CONFIG_WIFI_LONG_RETRY_DELAY_SECONDS));
    }
}

static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status)
    {
        LOG_ERR("Connection request failed (%d)", status->status);
        device_online = 0;
    }
    else
    {
        LOG_INF("Connected to WiFi" );
        device_online = 1;
        reconnecting = false;
        reconnect_attempts = 0;
        // cancel any pending reconnect work
        k_work_cancel_delayable(&wifi_reconnect_work);
        k_sem_give(&wifi_connected);
    }
}

static void handle_wifi_disconnect_result(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status)
    {
        LOG_INF("Disconnection request (%d)\n", status->status);
    }
    else{
        LOG_INF("Device is now disconnected\n");
        device_online = 0;
        k_sem_take(&wifi_connected, K_NO_WAIT);

        if (!reconnecting) {
            reconnecting = true;
            reconnect_attempts = 0;
            // schedule immediate attempt
            k_work_reschedule(&wifi_reconnect_work, K_NO_WAIT);
        }
    }
}

static void handle_ipv4_result(struct net_if *iface)
{
    int i = 0;

    for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {

        char buf[NET_IPV4_ADDR_LEN];

        if (iface->config.ip.ipv4->unicast[i].ipv4.addr_type != NET_ADDR_DHCP) {
            continue;
        }

        LOG_INF("IPv4 address: %s\n",
                net_addr_ntop(AF_INET,
                                &iface->config.ip.ipv4->unicast[i].ipv4.address.in_addr,
                                buf, sizeof(buf)));
        LOG_INF("Subnet: %s\n",
                net_addr_ntop(AF_INET,
                                &iface->config.ip.ipv4->unicast[i].netmask,
                                buf, sizeof(buf)));
        LOG_INF("Router: %s\n",
                net_addr_ntop(AF_INET,
                                &iface->config.ip.ipv4->gw,
                                buf, sizeof(buf)));
    }

    k_sem_give(&ipv4_address_obtained);
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event, struct net_if *iface)
{
    switch (mgmt_event)
    {

        case NET_EVENT_WIFI_CONNECT_RESULT:
            handle_wifi_connect_result(cb);
            break;

        case NET_EVENT_WIFI_DISCONNECT_RESULT:
            handle_wifi_disconnect_result(cb);
            break;

        case NET_EVENT_IPV4_ADDR_ADD:
            handle_ipv4_result(iface);
            break;

        default:
            break;
    }
}

int connect_wifi()
{
    struct net_if *iface = net_if_get_default();
    int err = 0;

    LOG_INF("Connecting to SSID: %s\n", wifi_params.ssid);
    if (s_retry_num < CONFIG_WIFI_MAX_RETRY_COUNT) {
        err = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &wifi_params, sizeof(struct wifi_connect_req_params));
        s_retry_num++;
        LOG_INF("Retry to connect to the AP\n");
    }
    if (err) {
        LOG_ERR("WiFi Connection Request Failed\n");
        return -1;
    }
    return 0;
}

void set_device_hostname() {
    struct net_if *iface = net_if_get_default();
    char hostname[32];

    if (iface == NULL) {
        LOG_ERR("No network interface found");
        return;
    }

    struct net_linkaddr *link_addr = net_if_get_link_addr(iface);

    if (link_addr == NULL || link_addr->len == 0) {
        LOG_ERR("No MAC address available");
        return;
    }

    // Format MAC address as string
#ifdef CONFIG_PROJECT_NAME
    char project_name[24] = {0};
    if (strlen(CONFIG_PROJECT_NAME) > 24) {
        strncpy(project_name, CONFIG_PROJECT_NAME, 24);
        project_name[24] = '\0';
    } else {
        strcpy(project_name, CONFIG_PROJECT_NAME);
    }
    snprintf(hostname, sizeof(hostname), "%s-%02X%02X%02X", project_name, link_addr->addr[3], link_addr->addr[4], link_addr->addr[5]);
#else
    snprintf(hostname, sizeof(hostname), "ESP32-%02X%02X%02X", link_addr->addr[3], link_addr->addr[4], link_addr->addr[5]);
#endif
    int ret = net_hostname_set(hostname, strlen(hostname));
    if (ret != 0) {
        LOG_ERR("Failed to set hostname: %d", ret);
    } else {
        LOG_INF("Hostname set to: %s", hostname);
    }
}

int init_wifi(char *wifi_ssid, char *wifi_password) {
    set_device_hostname();

    LOG_INF("Setting Wifi Client");

    if(wifi_ssid != NULL && wifi_password != NULL){

        size_t ssid_len = strlen(wifi_ssid);
        size_t psk_len = strlen(wifi_password);

        if(ssid_len == 0 || ssid_len > WIFI_SSID_MAX_LEN) {
            LOG_ERR("Invalid SSID length: %d (max: %d)", ssid_len, WIFI_SSID_MAX_LEN);
            return -EINVAL;
        }

        if(psk_len == 0 || psk_len > WIFI_PSK_MAX_LEN) {
            LOG_ERR("Invalid password length: %d (max: %d)", psk_len, WIFI_PSK_MAX_LEN);
            return -EINVAL;
        }

        wifi_params.ssid = (const uint8_t *)wifi_ssid;
        wifi_params.psk = (const uint8_t *)wifi_password;
        wifi_params.ssid_length = strlen(wifi_ssid);
        wifi_params.psk_length = strlen(wifi_password);
        wifi_params.channel = WIFI_CHANNEL_ANY;
        wifi_params.security = WIFI_SECURITY_TYPE_PSK;
        wifi_params.band = WIFI_FREQ_BAND_2_4_GHZ;
        wifi_params.mfp = WIFI_MFP_OPTIONAL;
    } else {
        LOG_ERR("Invalid WiFi credentials");
        return -EINVAL;
    }

    net_mgmt_init_event_callback(&wifi_cb, wifi_mgmt_event_handler,
                                 NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);

    net_mgmt_init_event_callback(&ipv4_cb, wifi_mgmt_event_handler, NET_EVENT_IPV4_ADDR_ADD);

    net_mgmt_add_event_callback(&wifi_cb);
    net_mgmt_add_event_callback(&ipv4_cb);
    k_work_init_delayable(&wifi_reconnect_work, reconnect_work_handler);

    connect_wifi();

    return 0;
}

int is_wifi_connected() {
    return device_online;
}