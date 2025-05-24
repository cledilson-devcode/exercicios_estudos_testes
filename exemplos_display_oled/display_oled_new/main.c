// main.c
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <string.h>

#include "inc/display/display_app.h"       // Para as funções da nossa aplicação de display

// Inclui o cabeçalho que define ssd1306_buffer_length e struct render_area
// Este provavelmente é "inc/ssd1306_i2c.h" no seu projeto original,
// ou "inc/ssd1306.h" se você tivesse seguido a refatoração completa.
#include "inc/display/ssd1306_i2c.h"

// Inclui o cabeçalho que define BlinkingAlertState
#include "inc/display/oled_messages.h"




#define BUTTON_GPIO 5

static uint8_t main_buffer_oled[ssd1306_buffer_length]; // ssd1306_buffer_length deve ser definido em ssd1306_i2c.h
static struct render_area main_area;                   // struct render_area deve ser definido em ssd1306_i2c.h
static BlinkingAlertState main_meu_alerta;             // BlinkingAlertState definido em oled_messages.h
static i2c_inst_t* main_i2c_port = i2c1;

static bool main_debounced_press_processed = false;
static uint32_t main_last_button_interaction_time = 0;
static bool main_last_raw_button_state = false;

// ... (função button_pressed_debounced_main como antes) ...
static bool button_pressed_debounced_main(uint pin, uint32_t *last_press_time_us, bool *last_button_state, bool *debounced_processed_flag) {
    const uint32_t debounce_delay_us = 50000;
    bool current_button_state = !gpio_get(pin);

    if (current_button_state != *last_button_state) {
        *last_press_time_us = time_us_32();
    }
    *last_button_state = current_button_state;

    if (current_button_state && (time_us_32() - *last_press_time_us > debounce_delay_us)) {
        if (!(*debounced_processed_flag)) {
            *debounced_processed_flag = true;
            return true;
        }
    } else if (!current_button_state) {
         *debounced_processed_flag = false;
    }
    return false;
}


int main() {
    stdio_init_all();

    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    if (!display_application_init(main_buffer_oled, &main_area, &main_meu_alerta, main_i2c_port)) {
        printf("Falha ao inicializar a aplicacao do display!\n");
        while(1);
    }

    printf("Aplicacao do display inicializada. Pressione o botao.\n");

    while (true) {
        if (button_pressed_debounced_main(BUTTON_GPIO, &main_last_button_interaction_time,
                                         &main_last_raw_button_state, &main_debounced_press_processed)) {
            printf("Botao pressionado - alternando alerta.\n");
            display_application_toggle_alert_state();
        }

        display_application_process();
        sleep_us(10000);
    }
    return 0;
}