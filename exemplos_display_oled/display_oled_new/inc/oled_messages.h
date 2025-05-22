#ifndef OLED_MESSAGES_H
#define OLED_MESSAGES_H


#include "ssd1306_i2c.h" // Para struct render_area e uint8_t
#include <stdint.h>      // Para uint32_t
#include <stdbool.h>     // Para bool
#include "pico/time.h"   // Para absolute_time_t

// Função para exibir uma série de mensagens de teste no OLED
void oled_display_test_messages(uint8_t *buffer, struct render_area *area);

// Função para exibir mensagens específicas com acentuação e aguardar
void oled_display_accent_test(uint8_t *buffer, struct render_area *area);

// Função para exibir uma mensagem piscando continuamente como alerta
// A mensagem será centralizada na linha_y especificada.
// interval_ms: tempo que a mensagem fica visível e tempo que fica invisível.
void oled_display_blinking_alert(uint8_t *buffer, struct render_area *area, const char *message, int16_t line_y_pixel, uint32_t interval_ms);



// ... (declarações de oled_display_test_messages e oled_display_accent_test) ...


// Estrutura para gerenciar o estado de uma mensagem piscante
typedef struct {
    const char *message;
    int16_t line_y_pixel;
    uint32_t interval_ms;
    bool visible;
    absolute_time_t last_toggle_time;
    bool initialized; // Para saber se já foi configurada
} BlinkingAlertState;

// Inicializa o estado do alerta piscante
void oled_alert_init(BlinkingAlertState *alert_state, const char *message, int16_t line_y, uint32_t interval_ms);

// Atualiza o display com o alerta piscante (deve ser chamada repetidamente no loop principal)
// Retorna true se o display foi atualizado, false caso contrário.
bool oled_alert_update(uint8_t *buffer, struct render_area *area, BlinkingAlertState *alert_state);

#endif // OLED_MESSAGES_H