#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/apps/httpd.h"
#include "lwipopts.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ui.h"

void ui_task(__unused void *params)
{
    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("failed to connect.\n");
        exit(1);
    }
    else
    {
        printf("Connected.\n");
    }

    httpd_init();
    printf("Http server initialised\n");

    while (true)
    {
        vTaskDelay(1000);
    }

    cyw43_arch_deinit();
}