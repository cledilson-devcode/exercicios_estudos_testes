#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h" // Para gpio_init, gpio_set_dir, gpio_get, gpio_pull_up
#include <string.h>

#include "inc/oled_setup.h"
#include "inc/ssd1306_i2c.h"
#include "inc/oled_messages.h"

#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15
#define FREQ_KHZ 400

#define BUTTON_GPIO 5 // Pino do botão

static bool debounced_press_processed = false;

static BlinkingAlertState meu_alerta;

// Simples debounce para o botão
bool button_pressed_debounced(uint pin, uint32_t *last_press_time_us, bool *last_button_state) {
    const uint32_t debounce_delay_us = 50000; // 50ms debounce
    bool current_button_state = !gpio_get(pin); // Invertido: true se pressionado (nível baixo)

    if (current_button_state != *last_button_state) {
        *last_press_time_us = time_us_32(); // Reseta o tempo do debounce
    }
    *last_button_state = current_button_state;

    if (current_button_state && (time_us_32() - *last_press_time_us > debounce_delay_us)) {
        *last_press_time_us = time_us_32(); // Evita múltiplas detecções rápidas
         // Para garantir que só dispare uma vez por pressionamento longo,
         // precisamos de uma lógica mais elaborada de "edge detection" após o debounce.
         // Por ora, este debounce simples já ajuda. Para um toggle, precisamos que ele
         // só dispare uma vez quando o estado muda de não pressionado para pressionado e debounced.

        // Para toggle, precisamos saber se já foi processado neste "pressionamento"
        
        if (!debounced_press_processed) {
            debounced_press_processed = true;
            return true;
        }

    } else if (!current_button_state) {
         // Reseta o flag de processamento quando o botão é solto
         debounced_press_processed = false;
    }
    return false;
}


int main() {
    stdio_init_all();

    uint8_t buffer_oled[ssd1306_buffer_length];
    struct render_area area;

    setup_display(I2C_PORT, SDA_PIN, SCL_PIN, FREQ_KHZ, buffer_oled, &area);

    // Configura o pino do botão
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO); // Habilita pull-up interno. Botão para GND quando pressionado.

    uint32_t last_button_interaction_time = 0; // Para debounce/controle de repetição
    bool last_raw_button_state = false; // Para debounce


    // Inicializa o alerta piscante (começa ativo por padrão)
    const char *mensagem_piscante = "AVISO";
    oled_alert_init(&meu_alerta, mensagem_piscante, 24, 500, true); // Mensagem, linha Y, intervalo, começa ativo

    // Força uma primeira renderização
    oled_clear(buffer_oled, &area);
    if (meu_alerta.active && meu_alerta.visible && meu_alerta.message) {
        int message_width_pixels = strlen(meu_alerta.message) * 8;
        int16_t start_x = (ssd1306_width - message_width_pixels) / 2;
        if (start_x < 0) start_x = 0;
        ssd1306_draw_string(buffer_oled, start_x, meu_alerta.line_y_pixel, meu_alerta.message);
    }
    render_on_display(buffer_oled, &area);


    while (true) {
        // Verifica o botão
        if (button_pressed_debounced(BUTTON_GPIO, &last_button_interaction_time, &last_raw_button_state)) {
            oled_alert_toggle_active(&meu_alerta, buffer_oled, &area);
        }

        // Atualiza o alerta piscante (só faz algo se estiver ativo)
        oled_alert_update(buffer_oled, &area, &meu_alerta);

        // Outras tarefas do loop principal podem vir aqui
        // sleep_us(10000); // Pequena pausa opcional
    }
    return 0;
}