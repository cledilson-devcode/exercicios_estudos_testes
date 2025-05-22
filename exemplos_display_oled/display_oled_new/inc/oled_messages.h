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




typedef struct {
    const char *message;
    int16_t line_y_pixel;
    uint32_t interval_ms;
    bool visible;
    absolute_time_t last_toggle_time;
    bool initialized;
    bool active; // Novo campo para controlar se o alerta está ativo
} BlinkingAlertState;

void oled_alert_init(BlinkingAlertState *alert_state, const char *message, int16_t line_y, uint32_t interval_ms, bool start_active);
bool oled_alert_update(uint8_t *buffer, struct render_area *area, BlinkingAlertState *alert_state);

// Funções para controlar o estado ativo do alerta
void oled_alert_activate(BlinkingAlertState *alert_state);
void oled_alert_deactivate(BlinkingAlertState *alert_state, uint8_t *buffer, struct render_area *area); // Precisa do buffer para limpar a tela
void oled_alert_toggle_active(BlinkingAlertState *alert_state, uint8_t *buffer, struct render_area *area);

// ... (declarações de oled_display_test_messages e oled_display_accent_test, se ainda usadas) ...

#endif // OLED_MESSAGES_H