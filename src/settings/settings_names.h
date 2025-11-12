//
// Created by efisio on 11/12/25.
//

#ifndef CYD_ZEPHYR_TESTS_SETTINGS_NAMES_H
#define CYD_ZEPHYR_TESTS_SETTINGS_NAMES_H

#define UUID_SZ         37
#define CONF_LINE_SIZE  64
#define MAX_ADV_NAME    20

typedef enum {
    PROJECT_NAME,
    WF_SSID,
    WF_PASS,
    PARAM_END
} ParamEnum;

typedef struct Settings {
    char project_name[20];
    char wifi_ssid[UUID_SZ];
    char wifi_pass[UUID_SZ];
} PersistentSettings;

extern PersistentSettings global_params;
#endif //CYD_ZEPHYR_TESTS_SETTINGS_NAMES_H