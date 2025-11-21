
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/net/sntp.h>
#include <zephyr/net/socket.h>
#include <time.h>
#include <lvgl.h>
#include "wifi_component.h"
#include "sntp_component.h"
#include "display_component.h"
#include "settings_component.h"
#include "settings_names.h"

LOG_MODULE_REGISTER(main);

int main(void) {

    (void)log_set_tag("cyd-zephyr-tests"); // Add once
    LOG_INF("App cyd-zephyr-tests started.");
    PersistentSettings global_params = {"ESP32", "fake_ap", "fake_pass"};
    PersistentSettings *global_params_p = &global_params;
    load_persistent_settings(global_params_p);


    init_wifi(global_params.wifi_ssid, global_params.wifi_pass, global_params.wifi_max_retry, global_params.wifi_short_retry_delay, global_params.wifi_long_retry_delay);

    lv_init();

    init_display();

    draw_content();

    while (true) {
        update_display();
    }

}

K_THREAD_DEFINE(ntp_thread_id, 2048, sync_time_periodically, NULL, NULL, NULL, 7, 0, 0);