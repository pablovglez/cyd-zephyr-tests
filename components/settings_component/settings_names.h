//
// Created by efisio on 11/12/25.
//

#ifndef SETTINGS_NAMES_H
#define SETTINGS_NAMES_H

#define UUID_SZ         37
#define CONF_LINE_SIZE  64
#define MAX_ADV_NAME    20

typedef enum {
    PROJECT_NAME,
    WF_SSID,
    WF_PASS,
    WF_MAX_RETRY,
    WF_SHORT_RETRY_DELAY,
    WF_LONG_RETRY_DELAY,
    PARAM_END
} ParamEnum;

typedef struct Settings {
    char project_name[20];
    char wifi_ssid[UUID_SZ];
    char wifi_pass[UUID_SZ];
    int wifi_max_retry;
    int wifi_short_retry_delay;
    int wifi_long_retry_delay;
} PersistentSettings;

#endif //CYD_ZEPHYR_TESTS_SETTINGS_NAMES_H