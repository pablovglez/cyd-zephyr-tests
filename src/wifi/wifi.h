//
// Created by efisio on 11/2/25.
//

#ifndef WIFI_H
#define WIFI_H

int connect_wifi();

int init_wifi(char *wifi_ssid, char *wifi_password);

int is_wifi_connected();

#endif //CYD_ZEPHYR_TESTS_WIFI_H