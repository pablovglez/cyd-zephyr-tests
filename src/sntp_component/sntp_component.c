#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/sntp.h>
#include <zephyr/net/socket.h>
#include "wifi_component/wifi_component.h"

LOG_MODULE_REGISTER(sntp);

#define SNTP_PORT 123
#define SNTP_SERVER "37.187.118.149" //"fr.pool.ntp.org"
#define MAX_RETRIES 3
#define RETRY_DELAY_MS 2000
#define QUERY_TIMEOUT_MS 5000

static int get_ntp_time(struct sntp_time *sntp_time)
{
    int ret;
    struct sntp_ctx ctx;
    struct sockaddr_in sntp_server;
    
    // Setup server address first
    sntp_server.sin_family = AF_INET;
    sntp_server.sin_port = htons(SNTP_PORT);
    
    // Resolve server address
    ret = net_ipaddr_parse(SNTP_SERVER, strlen(SNTP_SERVER), 
                          (struct sockaddr *)&sntp_server);
    if (ret <= 0) {
        LOG_ERR("Cannot parse SNTP server address: %s", SNTP_SERVER);
        return -EINVAL;
    }
    
    // Initialize SNTP context with server address
    ret = sntp_init(&ctx, (struct sockaddr *)&sntp_server, sizeof(sntp_server));
    if (ret < 0) {
        LOG_ERR("Failed to initialize SNTP context: %d", ret);
        return ret;
    }
    
    // Request time with retries
    for (int i = 0; i < MAX_RETRIES; i++) {
        ret = sntp_query(&ctx, QUERY_TIMEOUT_MS, sntp_time);
        if (ret == 0) {
            LOG_INF("SNTP query successful");
            break;
        }
        
        LOG_WRN("SNTP query failed (attempt %d/%d): %d", 
                i + 1, MAX_RETRIES, ret);
        
        if (i < MAX_RETRIES - 1) {
            k_msleep(RETRY_DELAY_MS);
        }
    }
    
    sntp_close(&ctx);
    return ret;
}

static void print_time(const struct sntp_time *sntp_time)
{
    uint64_t timestamp = sntp_time->seconds;
    
    // Convert to human readable time
    uint32_t year = 1970 + (timestamp % 31556952);
    
    LOG_INF("NTP Time received:");
    LOG_INF("  Timestamp: %llu seconds since 1970", timestamp);
}

void sync_time_once(void)
{
    int ret;
    struct sntp_time sntp_time;
    
    // Wait for network to be ready
    while (!is_wifi_connected()) {
        k_msleep(1000);
    }
    
    LOG_INF("Attempting to sync time with NTP server...");
    
    ret = get_ntp_time(&sntp_time);
    if (ret == 0) {
        print_time(&sntp_time);
        
        // You can set system time here if needed
        // For example, using clock_settime() if CONFIG_POSIX_CLOCK is enabled
        
        LOG_INF("Time synchronized successfully");
    } else {
        LOG_ERR("Failed to synchronize time: %d", ret);
    }
}

void sync_time_periodically(void)
{
    while (1) {
        sync_time_once();
        
        // Wait before next sync (e.g., every hour)
        k_sleep(K_HOURS(1));
    }
}
