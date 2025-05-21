#ifndef OLED_MESSAGES_H
#define OLED_MESSAGES_H

#include "ssd1306_i2c.h" // Para struct render_area e uint8_t

// Função para exibir uma série de mensagens de teste no OLED
void oled_display_test_messages(uint8_t *buffer, struct render_area *area);

// Função para exibir mensagens específicas com acentuação e aguardar
void oled_display_accent_test(uint8_t *buffer, struct render_area *area);

#endif // OLED_MESSAGES_H