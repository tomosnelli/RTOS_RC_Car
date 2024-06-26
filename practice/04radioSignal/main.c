/*
    practice radio signal handling
*/
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
// #include "hardware/spi.h"
#include "btstack.h"
#include "FreeRTOS.h"
#include "task.h"

#define LED_PIN 25

// BLE 
static uint8_t hid_service_buffer[ 270 ];
static uint8_t device_id_sdp_service_buffer[ 100 ];
static const char hid_device_name[] = "DualSense Wireless Controller";
static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint16_t hid_cid;

const uint8_t hid_descriptor_ctrl_mode[] = 
{
    0x05, 0x01,                    //  Usage Page (Generic Desktop)
    0x09, 0x05,                    //  Usage (Game Pad)
    0xa1, 0x01,                    //  Collection (Application)
    0x85, 0x01,                    //  Report ID (1)                      6
    0x09, 0x30,                    //  Usage (X)                          8
    0x09, 0x31,                    //  Usage (Y)                          10
    0x09, 0x32,                    //  Usage (Z)                          12
    0x09, 0x35,                    //  Usage (Rz)                         14
    0x15, 0x00,                    //  Logical Minimum (0)                16
    0x26, 0xff, 0x00,              //  Logical Maximum (255)              18
    0x75, 0x08,                    //  Report Size (8)                    21
    0x95, 0x04,                    //  Report Count (4)                   23
    0x81, 0x02,                    //  Input (Data,Var,Abs)               25
    0x09, 0x39,                    //  Usage (Hat switch)                 27
    0x15, 0x00,                    //  Logical Minimum (0)                29
    0x25, 0x07,                    //  Logical Maximum (7)                31
    0x35, 0x00,                    //  Physical Minimum (0)               33
    0x46, 0x3b, 0x01,              //  Physical Maximum (315)             35
    0x65, 0x14,                    //  Unit (EnglishRotation: deg)        38
    0x75, 0x04,                    //  Report Size (4)                    40
    0x95, 0x01,                    //  Report Count (1)                   42
    0x81, 0x42,                    //  Input (Data,Var,Abs,Null)          44
    0x65, 0x00,                    //  Unit (None)                        46
    0x05, 0x09,                    //  Usage Page (Button)                48
    0x19, 0x01,                    //  Usage Minimum (1)                  50
    0x29, 0x0e,                    //  Usage Maximum (14)                 52
    0x15, 0x00,                    //  Logical Minimum (0)                54
    0x25, 0x01,                    //  Logical Maximum (1)                56
    0x75, 0x01,                    //  Report Size (1)                    58
    0x95, 0x0e,                    //  Report Count (14)                  60
    0x81, 0x02,                    //  Input (Data,Var,Abs)               62
    0x75, 0x06,                    //  Report Size (6)                    64
    0x95, 0x01,                    //  Report Count (1)                   66
    0x81, 0x01,                    //  Input (Cnst,Arr,Abs)               68
    0x05, 0x01,                    //  Usage Page (Generic Desktop)       70
    0x09, 0x33,                    //  Usage (Rx)                         72
    0x09, 0x34,                    //  Usage (Ry)                         74
    0x15, 0x00,                    //  Logical Minimum (0)                76
    0x26, 0xff, 0x00,              //  Logical Maximum (255)              78
    0x75, 0x08,                    //  Report Size (8)                    81
    0x95, 0x02,                    //  Report Count (2)                   83
    0x81, 0x02,                    //  Input (Data,Var,Abs)               85
    0x06, 0x00, 0xff,              //  Usage Page (Vendor Defined Page 1) 87
    0x15, 0x00,                    //  Logical Minimum (0)                90
    0x26, 0xff, 0x00,              //  Logical Maximum (255)              92
    0x75, 0x08,                    //  Report Size (8)                    95
    0x95, 0x4d,                    //  Report Count (77)                  97
    0x85, 0x31,                    //  Report ID (49)                     99
    0x09, 0x31,                    //  Usage (Vendor Usage 0x31)          101
    0x91, 0x02,                    //  Output (Data,Var,Abs)              103
    0x09, 0x3b,                    //  Usage (Vendor Usage 0x3b)          105
    0x81, 0x02,                    //  Input (Data,Var,Abs)               107
    0x85, 0x32,                    //  Report ID (50)                     109
    0x09, 0x32,                    //  Usage (Vendor Usage 0x32)          111
    0x95, 0x8d,                    //  Report Count (141)                 113
    0x91, 0x02,                    //  Output (Data,Var,Abs)              115
    0x85, 0x33,                    //  Report ID (51)                     117
    0x09, 0x33,                    //  Usage (Vendor Usage 0x33)          119
    0x95, 0xcd,                    //  Report Count (205)                 121
    0x91, 0x02,                    //  Output (Data,Var,Abs)              123
    0x85, 0x34,                    //  Report ID (52)                     125
    0x09, 0x34,                    //  Usage (Vendor Usage 0x34)          127
    0x96, 0x0d, 0x01,              //  Report Count (269)                 129
    0x91, 0x02,                    //  Output (Data,Var,Abs)              132
    0x85, 0x35,                    //  Report ID (53)                     134
    0x09, 0x35,                    //  Usage (Vendor Usage 0x35)          136
    0x96, 0x4d, 0x01,              //  Report Count (333)                 138
    0x91, 0x02,                    //  Output (Data,Var,Abs)              141
    0x85, 0x36,                    //  Report ID (54)                     143
    0x09, 0x36,                    //  Usage (Vendor Usage 0x36)          145
    0x96, 0x8d, 0x01,              //  Report Count (397)                 147
    0x91, 0x02,                    //  Output (Data,Var,Abs)              150
    0x85, 0x37,                    //  Report ID (55)                     152
    0x09, 0x37,                    //  Usage (Vendor Usage 0x37)          154
    0x96, 0xcd, 0x01,              //  Report Count (461)                 156
    0x91, 0x02,                    //  Output (Data,Var,Abs)              159
    0x85, 0x38,                    //  Report ID (56)                     161
    0x09, 0x38,                    //  Usage (Vendor Usage 0x38)          163
    0x96, 0x0d, 0x02,              //  Report Count (525)                 165
    0x91, 0x02,                    //  Output (Data,Var,Abs)              168
    0x85, 0x39,                    //  Report ID (57)                     170
    0x09, 0x39,                    //  Usage (Vendor Usage 0x39)          172
    0x96, 0x22, 0x02,              //  Report Count (546)                 174
    0x91, 0x02,                    //  Output (Data,Var,Abs)              177
    0x06, 0x80, 0xff,              //  Usage Page (Vendor Usage Page 0xff80) 179
    0x85, 0x05,                    //  Report ID (5)                      182
    0x09, 0x33,                    //  Usage (Vendor Usage 0x33)          184
    0x95, 0x28,                    //  Report Count (40)                  186
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             188
    0x85, 0x08,                    //  Report ID (8)                      190
    0x09, 0x34,                    //  Usage (Vendor Usage 0x34)          192
    0x95, 0x2f,                    //  Report Count (47)                  194
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             196
    0x85, 0x09,                    //  Report ID (9)                      198
    0x09, 0x24,                    //  Usage (Vendor Usage 0x24)          200
    0x95, 0x13,                    //  Report Count (19)                  202
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             204
    0x85, 0x20,                    //  Report ID (32)                     206
    0x09, 0x26,                    //  Usage (Vendor Usage 0x26)          208
    0x95, 0x3f,                    //  Report Count (63)                  210
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             212
    0x85, 0x22,                    //  Report ID (34)                     214
    0x09, 0x40,                    //  Usage (Vendor Usage 0x40)          216
    0x95, 0x3f,                    //  Report Count (63)                  218
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             220
    0x85, 0x80,                    //  Report ID (128)                    222
    0x09, 0x28,                    //  Usage (Vendor Usage 0x28)          224
    0x95, 0x3f,                    //  Report Count (63)                  226
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             228
    0x85, 0x81,                    //  Report ID (129)                    230
    0x09, 0x29,                    //  Usage (Vendor Usage 0x29)          232
    0x95, 0x3f,                    //  Report Count (63)                  234
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             236
    0x85, 0x82,                    //  Report ID (130)                    238
    0x09, 0x2a,                    //  Usage (Vendor Usage 0x2a)          240
    0x95, 0x09,                    //  Report Count (9)                   242
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             244
    0x85, 0x83,                    //  Report ID (131)                    246
    0x09, 0x2b,                    //  Usage (Vendor Usage 0x2b)          248
    0x95, 0x3f,                    //  Report Count (63)                  250
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             252
    0x85, 0xf1,                    //  Report ID (241)                    254
    0x09, 0x31,                    //  Usage (Vendor Usage 0x31)          256
    0x95, 0x3f,                    //  Report Count (63)                  258
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             260
    0x85, 0xf2,                    //  Report ID (242)                    262
    0x09, 0x32,                    //  Usage (Vendor Usage 0x32)          264
    0x95, 0x0f,                    //  Report Count (15)                  266
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             268
    0x85, 0xf0,                    //  Report ID (240)                    270
    0x09, 0x30,                    //  Usage (Vendor Usage 0x30)          272
    0x95, 0x3f,                    //  Report Count (63)                  274
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             276
    0x85, 0xf4,                    //  Report ID (244)                    278
    0x09, 0x2c,                    //  Usage (Vendor Usage 0x2c)          280
    0x95, 0x3f,                    //  Report Count (63)                  282
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             284
    0x85, 0xf5,                    //  Report ID (245)                    286
    0x09, 0x2d,                    //  Usage (Vendor Usage 0x2d)          288
    0x95, 0x07,                    //  Report Count (7)                   290
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             292
    0x85, 0xf6,                    //  Report ID (246)                    294
    0x09, 0x2e,                    //  Usage (Vendor Usage 0x2e)          296
    0x96, 0x22, 0x02,              //  Report Count (546)                 298
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             301
    0x85, 0xf7,                    //  Report ID (247)                    303
    0x09, 0x2f,                    //  Usage (Vendor Usage 0x2f)          305
    0x95, 0x07,                    //  Report Count (7)                   307
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             309
    0xc0,                          // End Collection                      311
};

void led_task( void* pvParameters ) {
    ( void ) pvParameters;
    gpio_init( PICO_DEFAULT_LED_PIN );
    gpio_set_dir( PICO_DEFAULT_LED_PIN, GPIO_OUT );

    while ( 1 ) {
        gpio_put( LED_PIN, 1 ); // Turn on LED
        vTaskDelay( pdMS_TO_TICKS( 500 ) ); // Delay 500 milliseconds
        gpio_put( LED_PIN, 0 ); // Turn off LED
        vTaskDelay(pdMS_TO_TICKS( 500 ) ); // Delay 500 milliseconds
    }
}

int main() {
    stdio_init_all();

    // Create tasks for LED blinking and radio signal reception
    xTaskCreate( led_task, "LED Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // SHOULD NEVER COME HERE
    for ( ; ; );

    return 0;
}
