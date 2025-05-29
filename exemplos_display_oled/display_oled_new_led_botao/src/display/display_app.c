// src/display_app.c
#include "inc/display/display_app.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h>

#include "inc/display/oled_setup.h"
#include "inc/display/ssd1306_i2c.h" // Ou o .h que define ssd1306_buffer_length, etc.
// oled_messages.h já é incluído por display_app.h

// --- Variáveis estáticas para manter o estado da aplicação ---
static uint8_t *s_app_buffer_oled = NULL;
static struct render_area *s_app_area = NULL;
static BlinkingAlertState *s_app_alert_state = NULL;
static i2c_inst_t *s_app_i2c_port = NULL;

// Definições
#define APP_I2C_PORT i2c1
#define APP_SDA_PIN 14
#define APP_SCL_PIN 15
#define APP_FREQ_KHZ 400

#define LED_VERMELHO_PIN 13 // <<< NOVO: Defina o pino GPIO para o LED vermelho

bool display_application_init(uint8_t *buffer_ptr, struct render_area *area_ptr,
                              BlinkingAlertState *alert_state_ptr, i2c_inst_t *i2c_port_ptr) {
    if (!buffer_ptr || !area_ptr || !alert_state_ptr || !i2c_port_ptr) {
        return false;
    }

    s_app_buffer_oled = buffer_ptr;
    s_app_area = area_ptr;
    s_app_alert_state = alert_state_ptr;
    s_app_i2c_port = i2c_port_ptr;

    // Inicialização do display
    setup_display(APP_I2C_PORT, APP_SDA_PIN, APP_SCL_PIN, APP_FREQ_KHZ,
                  s_app_buffer_oled, s_app_area);

    // <<< NOVO: Inicialização do pino do LED vermelho >>>
    gpio_init(LED_VERMELHO_PIN);
    gpio_set_dir(LED_VERMELHO_PIN, GPIO_OUT);
    gpio_put(LED_VERMELHO_PIN, 0); // Começa desligado

    // Inicializa o alerta
    // A função oled_alert_init em oled_messages.c precisará ser ciente do LED
    // ou as funções de toggle e update em oled_messages.c controlarão o LED diretamente.
    // Vamos fazer com que oled_messages.c controle o LED.
    oled_alert_init(s_app_alert_state,
                    "EVACUAR!!!",
                    "SEGURO",
                    24,
                    500,
                    true, // Começa ativo
                    s_app_i2c_port,
                    s_app_buffer_oled,
                    s_app_area,
                    LED_VERMELHO_PIN); // <<< NOVO: Passando o pino do LED

    return true;
}

void display_application_toggle_alert_state(void) {
    if (s_app_buffer_oled && s_app_area && s_app_alert_state && s_app_i2c_port) {
        // oled_alert_toggle_active agora também precisa do pino do LED
        oled_alert_toggle_active(s_app_alert_state, s_app_buffer_oled, s_app_area, s_app_i2c_port, LED_VERMELHO_PIN);
    }
}

void display_application_process(void) {
    if (s_app_buffer_oled && s_app_area && s_app_alert_state && s_app_i2c_port) {
        // oled_alert_update agora também precisa do pino do LED
        oled_alert_update(s_app_buffer_oled, s_app_area, s_app_i2c_port, s_app_alert_state, LED_VERMELHO_PIN);
    }
}