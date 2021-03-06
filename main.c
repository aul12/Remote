//
// Created by paul on 20.01.18.
//
#include <avr/io.h>
#include <avr/interrupt.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#include "HAL/adc.h"
#include "HAL/uart.h"
#include "Drivers/rcLib/rc_lib.h"
#include "Util/Controller/joystick.h"
#include "Util/Controller/controller.h"
#include "Util/Model/model.h"
#include "Drivers/sx127x.h"
#include "HAL/timer8bit.h"

#define MUX_BASE 8

static volatile rc_lib_package_t *pkg_to_send_lora = NULL;
static volatile uint8_t send_event;

void uart_callback(uint8_t data) {
    static rc_lib_package_t pkg_uart_in;
    if(rc_lib_decode(&pkg_uart_in, data)) {
        *pkg_to_send_lora = pkg_uart_in;
    }
}

void timer_event() {
    static uint8_t mux = 0;
    mux += 1;
    if (mux % MUX_BASE == 0 && mux > 0) {
        send_event = mux / MUX_BASE;
        if (send_event >= 4) {
            mux = 0;
        }
    }
}

int main(void) {
    cli();
    adc_init();
    uart_init(0, 115200, NONE, 1, &uart_callback);

    joystick_init(&joystick_left);
    joystick_init(&joystick_right);
    joystick_load_calibration(&joystick_left, 0);
    joystick_load_calibration(&joystick_right, 16);

    sei();

    controller_init();
    model_init();
    rc_lib_transmitter_id = 17;
    rc_lib_package_t pkg_out, pkg_lora_in;
    rc_lib_init_rx(&pkg_lora_in);
    rc_lib_init_tx(&pkg_out, 256, 8);

    timer_8bit_init(prescaler_1024, &timer_event);

    while(true) {
        joystick_set_x_value(&joystick_left, adc_read_sync(1));
        joystick_set_y_value(&joystick_left, adc_read_sync(2));
        joystick_set_x_value(&joystick_right, adc_read_sync(4));
        joystick_set_y_value(&joystick_right, adc_read_sync(5));

        pkg_out.channel_data[0] = (uint16_t) (-joystick_get_x_value(&joystick_right) + 127);
        pkg_out.channel_data[1] = (uint16_t) (joystick_get_y_value(&joystick_right) + 127);
        pkg_out.channel_data[2] = (uint16_t) (-joystick_get_x_value(&joystick_left) + 127);
        pkg_out.channel_data[3] = (uint16_t) (joystick_get_y_value(&joystick_left) + 127);
        pkg_out.channel_data[4] = model_flightmode;
        pkg_out.channel_data[5] = model_armed;
        pkg_out.channel_data[6] = 0;
        pkg_out.channel_data[7] = -model_rssi;

        uint8_t curr_send_event = send_event;
        send_event = 0;

        switch (curr_send_event) {
            case 0:
                controller_update();
                break;
            case 1:
                if (model_get_serial_enabled()) {
                    uint8_t out_len = rc_lib_encode(&pkg_out);
                    uart_send_buf(0, pkg_out.buffer, out_len);
                }
                break;
            case 2:
                if (sx127x_package_available()) {
                    uint8_t buf[128];
                    uint8_t len = sx127x_wait_for_package(buf, 128);

                    model_rssi = sx127x_rssi();
                    model_snr = sx127x_snr();

                    for (uint8_t b = 0; b < len; ++b) {
                        if (rc_lib_decode(&pkg_lora_in, buf[b])) {
                            model_remote_rssi = -pkg_lora_in.channel_data[0];

                            uint8_t channel_min = pkg_lora_in.channel_count < 8 ? pkg_lora_in.channel_count : 8;
                            for (uint8_t c = 0; c < channel_min; ++c) {
                                model_receive_data[c] = pkg_lora_in.channel_data[c];
                            }

                            model_received++;
                        }
                        uart_send_byte(0, buf[b]);
                    }
                }
                break;
            case 3:
                {
                    uint8_t out_len = rc_lib_encode(&pkg_out);
                    sx127x_transmit(pkg_out.buffer, out_len);
                    model_sent++;
                }
                break;
            case 4:
                if (pkg_to_send_lora != NULL) {
                    uint16_t len = rc_lib_encode(pkg_to_send_lora);
                    sx127x_transmit(pkg_to_send_lora->buffer, len);
                    model_sent++;
                    pkg_to_send_lora = NULL;
                }
                break;
            default: break;
        }
    }
}

#pragma clang diagnostic pop
