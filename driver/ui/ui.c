#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "lwip/apps/httpd.h"
#include "lwipopts.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ui.h"
#include "driver/magnetometer/magnetometer.h"


// CGI handler which is run when a request for /led.cgi is detected
const char *cgi_led_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // Check if an request for LED has been made (/led.cgi?led=x)
    if (strcmp(pcParam[0], "led") == 0)
    {
        // Look at the argument to check if LED is to be turned on (x=1) or off (x=0)
        if (strcmp(pcValue[0], "0") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        else if (strcmp(pcValue[0], "1") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    }

    // Send the index page back to the user
    return "/index.shtml";
}

// tCGI Struct
// Fill this with all of the CGI requests and their respective handlers
static const tCGI cgi_handlers[] = {
    {// Html request for "/led.cgi" triggers cgi_handler
     "/led.cgi", cgi_led_handler},
};

void cgi_init(void)
{
    http_set_cgi_handlers(cgi_handlers, 1);
}


// SSI tags - tag length limited to 8 bytes by default
const char *ssi_tags[] = {"volt", "temp", "led", "comp"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen)
{
    size_t printed;
    switch (iIndex)
    {
    case 0: // volt
    {
        const float voltage = adc_read() * 3.3f / (1 << 12);
        printed = snprintf(pcInsert, iInsertLen, "%f", voltage);
    }
    break;
    case 1: // temp
    {
        const float voltage = adc_read() * 3.3f / (1 << 12);
        const float tempC = 27.0f - (voltage - 0.706f) / 0.001721f;
        printed = snprintf(pcInsert, iInsertLen, "%f", tempC);
    }
    break;
    case 2: // led
    {
        bool led_status = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
        if (led_status == true)
        {
            printed = snprintf(pcInsert, iInsertLen, "ON");
        }
        else
        {
            printed = snprintf(pcInsert, iInsertLen, "OFF");
        }
    }
    break;
    case 3: // compass
    {
        const float compass_reading = compass_read_degrees();
        printf("compass reading on ui:: %f\n", compass_reading);
        printed = snprintf(pcInsert, iInsertLen, "%f", compass_reading);
    }
    break;
    default:
        printed = 0;
        break;
    }

    return (u16_t)printed;
}

// Initialise the SSI handler
void ssi_init()
{
    // Initialise ADC (internal pin)
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}


void ui_task(__unused void *params)
{
    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms("Pixel_8072", "adafbb6699", CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("failed to connect.\n");
    }
    else
    {
        printf("Connected.\n");
    }

    httpd_init();
    printf("Http server initialised\n");

    // Configure SSI and CGI handler
    ssi_init();
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");

    while (true)
    {
        // vTaskDelay(1000);
    }

    cyw43_arch_deinit();
}