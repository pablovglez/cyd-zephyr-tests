
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <lvgl.h>
#include "wifi/wifi.h"
//#include "display/display_touch.h"
#include "display/display.h"
#include "settings/settings.h"

LOG_MODULE_REGISTER(main);

int main(void) {

    // Enable touch display function from display_touch.c
    /*
    int ret = 0;
    ret=draw_content_touch();

    return ret;
*/
    (void)log_set_tag("cyd-zephyr-tests"); // Add once
    LOG_INF("App cyd-zephyr-tests started.");
    load_persistent_settings();

#ifdef CONFIG_WIFI_SSID
    init_wifi(CONFIG_WIFI_SSID, CONFIG_WIFI_PSK);
#endif
    lv_init();

    init_display();

    draw_content();

    while (true) {
        update_display();
    }

}