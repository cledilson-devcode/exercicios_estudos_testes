#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>

#include "oled_setup.h"       // <- novo cabeçalho da função modular
#include "ssd1306_utils.h"
#include "ssd1306_text.h"

#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15
#define FREQ_KHZ 400

int main() {
    stdio_init_all();

    uint8_t buffer_oled[ssd1306_buffer_length];
    struct render_area area;

    // Inicialização modular do OLED
    setup_display(I2C_PORT, SDA_PIN, SCL_PIN, FREQ_KHZ, buffer_oled, &area);

    // Exibe textos de teste
    char msg[7] = {0x23, 0x23, 0x23, 0x23, 0x23, 0x23, '\0'};
    ssd1306_draw_string(buffer_oled, 0, 0, msg);
    ssd1306_draw_string(buffer_oled, 64, 0, msg);

    ssd1306_draw_string(buffer_oled, 10, 16, "Texto no OLED!");
    ssd1306_draw_string(buffer_oled, 0, 24, "ABCDEFGHIJKLMNO");
    ssd1306_draw_string(buffer_oled, 0, 32, "PQRSTUVWXZ");
    ssd1306_draw_string(buffer_oled, 0, 40, "0123456789");
    ssd1306_draw_string(buffer_oled, 0, 48, "abcdefghijklmno");
    ssd1306_draw_string(buffer_oled, 0, 56, "pqrstuvwxz");

    render_on_display(buffer_oled, &area);

    limpar_oled_com_delay(buffer_oled, &area, 2000);
    exibir_e_esperar(buffer_oled, &area, "#!:,.çá", 0);

    while (true) {
        sleep_ms(1000);
    }
}
