#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/sntp.h>
#include <zephyr/net/socket.h>
#include "wifi_component.h"

LOG_MODULE_REGISTER(sntp);

#define SNTP_PORT 123
#define SNTP_SERVER "37.187.118.149" //"fr.pool.ntp.org"
#define MAX_RETRIES 3
#define RETRY_DELAY_MS 2000
#define QUERY_TIMEOUT_MS 5000

static struct k_mutex sntp_mutex;


static int get_ntp_time(struct sntp_time *sntp_time)
{
    int ret;
    
    k_mutex_lock(&sntp_mutex, K_FOREVER);
    
    for (int i = 0; i < MAX_RETRIES; i++) {
        ret = sntp_simple(SNTP_SERVER, QUERY_TIMEOUT_MS, sntp_time);
        
        if (ret == 0) {
            LOG_INF("SNTP sync successful");
            break;
        } else {
            LOG_WRN("SNTP query failed (attempt %d/%d): %d", 
                    i + 1, MAX_RETRIES, ret);
            
            if (i < MAX_RETRIES - 1) {
                k_msleep(RETRY_DELAY_MS);
            }
        }
    }
    
    k_mutex_unlock(&sntp_mutex);
    return ret;
}

void sync_time_once(void)
{
    int ret;
    struct sntp_time sntp_time;
    
    // Wait for network to be ready
    while (!is_wifi_connected()) {
        LOG_INF("Waiting for WiFi connection...");
        k_msleep(15000);
    }
    
    LOG_INF("Attempting to sync time with NTP server...");
    
    ret = get_ntp_time(&sntp_time);
    if (ret == 0) {
        LOG_DBG("Timestamp: %llu seconds since 1970", sntp_time.seconds);
                
        // Set system time
            struct timespec tp = {
                .tv_sec = sntp_time.seconds,
                .tv_nsec = (sntp_time.fraction * 1000000000ULL) / 4294967296ULL
            };

            if (sys_clock_settime(CLOCK_REALTIME, &tp) == 0) {
                LOG_INF("System time updated successfully");
            } else {
                LOG_ERR("Failed to set system time");
            }

    } else {
        LOG_ERR("Failed to synchronize time: %d", ret);
    }
}

void sync_time_periodically(void)
{
    while (1) {
        sync_time_once();       
        // Wait before next sync
        k_sleep(K_HOURS(1));
    }
}
