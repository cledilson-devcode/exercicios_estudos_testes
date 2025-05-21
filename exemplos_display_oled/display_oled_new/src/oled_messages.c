#include "inc/oled_messages.h"
#include "inc/ssd1306_text.h"  // Para ssd1306_draw_string, exibir_e_esperar
#include "inc/ssd1306_utils.h" // Para limpar_oled_com_delay, render_on_display, oled_clear
#include "pico/stdlib.h"       // Para sleep_ms

// Função para exibir as mensagens de teste iniciais
void oled_display_test_messages(uint8_t *buffer, struct render_area *area) {
    // Limpa o buffer antes de desenhar novas mensagens
    oled_clear(buffer, area);

    char msg_hashes[7] = {0x23, 0x23, 0x23, 0x23, 0x23, 0x23, '\0'}; // "######"
    ssd1306_draw_string(buffer, 0, 0, msg_hashes);
    ssd1306_draw_string(buffer, 64, 0, msg_hashes);

    ssd1306_draw_string(buffer, 10, 16, "Texto no OLED!");
    ssd1306_draw_string(buffer, 0, 24, "ABCDEFGHIJKLMNO");
    ssd1306_draw_string(buffer, 0, 32, "PQRSTUVWXZ");
    ssd1306_draw_string(buffer, 0, 40, "0123456789");
    ssd1306_draw_string(buffer, 0, 48, "abcdefghijklmno");
    ssd1306_draw_string(buffer, 0, 56, "pqrstuvwxz");

    render_on_display(buffer, area); // Mostra tudo no display
}

// Função para exibir mensagens de teste com acentos e pausas
void oled_display_accent_test(uint8_t *buffer, struct render_area *area) {
    // A função limpar_oled_com_delay já limpa e renderiza a tela vazia.
    limpar_oled_com_delay(buffer, area, 2000); // Espera 2s com tela limpa

    // exibir_e_esperar limpa, desenha, renderiza e a pausa é controlada externamente.
    exibir_e_esperar(buffer, area, "Olá Pico! Çç Áá.", 0);
    render_on_display(buffer, area); // Garante que foi exibido
    sleep_ms(2000); // Pausa após exibir a primeira mensagem acentuada

    exibir_e_esperar(buffer, area, "Acentos: Ãã Êê Íí Óó Úú.\nTeste de quebra.", 0);
    render_on_display(buffer, area); // Garante que foi exibido
    // A última mensagem ficará na tela até o próximo clear ou ação.
}