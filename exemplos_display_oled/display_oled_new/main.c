#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>

#include "inc/oled_setup.h"
#include "inc/ssd1306_i2c.h"
#include "inc/oled_messages.h" // Inclui a struct BlinkingAlertState e as novas funções

#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15
#define FREQ_KHZ 400

// Variável global para o estado do alerta, ou pode ser passada
static BlinkingAlertState meu_alerta;

int main() {
    stdio_init_all();

    uint8_t buffer_oled[ssd1306_buffer_length];
    struct render_area area;

    setup_display(I2C_PORT, SDA_PIN, SCL_PIN, FREQ_KHZ, buffer_oled, &area);

    // Exemplo de como usar as funções de teste anteriores (opcional)
    /*
    oled_display_test_messages(buffer_oled, &area);
    sleep_ms(2000);
    oled_display_accent_test(buffer_oled, &area);
    sleep_ms(2000);
    */

    // Inicializa o estado do alerta piscante
    const char *mensagem_piscante = "ATENCAO";
    oled_alert_init(&meu_alerta, mensagem_piscante, 24, 750); // Mensagem, linha Y 24, pisca a cada 750ms

    // Força uma primeira renderização para que a mensagem apareça imediatamente se 'visible' for true
    // oled_clear(buffer_oled, &area);
    // if (meu_alerta.visible) {
    //     int message_width_pixels = strlen(meu_alerta.message) * 8;
    //     int16_t start_x = (ssd1306_width - message_width_pixels) / 2;
    //     if (start_x < 0) start_x = 0;
    //     ssd1306_draw_string(buffer_oled, start_x, meu_alerta.line_y_pixel, meu_alerta.message);
    // }
    // render_on_display(buffer_oled, &area);


    //uint32_t counter = 0; // Exemplo de outra tarefa rodando

    while (true) {
        // Atualiza o alerta piscante
        oled_alert_update(buffer_oled, &area, &meu_alerta);

        // Aqui você pode fazer outras coisas no seu loop principal
        // Por exemplo, ler sensores, piscar um LED onboard, etc.
        // Este printf é apenas um exemplo de tarefa não bloqueante.
        /*
        if ((counter % 100000) == 0) { // Imprime a cada X iterações para não inundar o serial
             printf("Loop principal rodando... Contador: %lu\n", counter / 100000);
        }
        counter++;
        */
        
        // Pequena pausa para não sobrecarregar o processador desnecessariamente,
        // mas o suficiente para a piscada ser responsiva.
        // O tempo de piscada é controlado pelo 'interval_ms' do alerta.
        // sleep_us(10000); // 10ms. Ajuste conforme necessário.
                          // Se o intervalo de piscada for grande, esta pausa pode ser maior.
                          // Se o intervalo for pequeno, esta pausa deve ser menor ou removida
                          // se o resto do loop levar tempo suficiente.
    }
    return 0;
}