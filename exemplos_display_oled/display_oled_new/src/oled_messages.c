
#include "inc/oled_messages.h"
#include "inc/ssd1306_text.h"
#include "inc/ssd1306_utils.h"
#include "inc/ssd1306_i2c.h"
#include "pico/stdlib.h"
#include "pico/time.h" // Para get_absolute_time, absolute_time_diff_us
#include <string.h>



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






// ... (implementações de oled_display_test_messages e oled_display_accent_test, se ainda usadas) ...

// Inicializa o estado de uma mensagem de alerta piscante
void oled_alert_init(BlinkingAlertState *alert_state, const char *message, int16_t line_y, uint32_t interval_ms, bool start_active) {
    if (!alert_state) return;
    alert_state->message = message;
    alert_state->line_y_pixel = line_y;
    alert_state->interval_ms = interval_ms;
    alert_state->visible = true; // Começa visível se ativo
    alert_state->last_toggle_time = get_absolute_time();
    alert_state->active = start_active;
    alert_state->initialized = true;
}

// Ativa o alerta
void oled_alert_activate(BlinkingAlertState *alert_state) {
    if (alert_state && alert_state->initialized) {
        alert_state->active = true;
        alert_state->visible = true; // Ao reativar, começa visível
        alert_state->last_toggle_time = get_absolute_time(); // Reseta o timer da piscada
    }
}

// Desativa o alerta e limpa a mensagem da tela
void oled_alert_deactivate(BlinkingAlertState *alert_state, uint8_t *buffer, struct render_area *area) {
    if (alert_state && alert_state->initialized) {
        alert_state->active = false;
        // Limpa a área onde a mensagem estava (ou a tela inteira)
        oled_clear(buffer, area);
        render_on_display(buffer, area);
    }
}

// Alterna o estado do alerta (ativo/inativo)
void oled_alert_toggle_active(BlinkingAlertState *alert_state, uint8_t *buffer, struct render_area *area) {
    if (alert_state && alert_state->initialized) {
        if (alert_state->active) {
            oled_alert_deactivate(alert_state, buffer, area);
        } else {
            oled_alert_activate(alert_state);
            // Força uma renderização inicial ao ativar
            oled_clear(buffer, area);
            if (alert_state->visible && alert_state->message) { // Verifica se message não é NULL
                 int message_width_pixels = strlen(alert_state->message) * 8;
                 int16_t start_x = (ssd1306_width - message_width_pixels) / 2;
                 if (start_x < 0) start_x = 0;
                 ssd1306_draw_string(buffer, start_x, alert_state->line_y_pixel, alert_state->message);
            }
            render_on_display(buffer, area);
        }
    }
}


// Atualiza o display com o alerta piscante. Deve ser chamada repetidamente.
bool oled_alert_update(uint8_t *buffer, struct render_area *area, BlinkingAlertState *alert_state) {
    if (!alert_state || !alert_state->initialized || !alert_state->message) {
        return false;
    }

    // Se o alerta não estiver ativo, não faz nada para piscar
    if (!alert_state->active) {
        // Poderíamos verificar se estava visível antes de desativar e limpar apenas uma vez,
        // mas o oled_alert_deactivate já cuida de limpar a tela.
        return false;
    }

    absolute_time_t current_time = get_absolute_time();
    int64_t diff_us = absolute_time_diff_us(alert_state->last_toggle_time, current_time);
    uint64_t interval_us = (uint64_t)alert_state->interval_ms * 1000;

    bool needs_display_refresh = false;

    if (diff_us >= interval_us) {
        alert_state->visible = !alert_state->visible;
        alert_state->last_toggle_time = current_time;
        needs_display_refresh = true;
    }

    if (needs_display_refresh) {
        oled_clear(buffer, area);

        if (alert_state->visible) {
            int message_width_pixels = strlen(alert_state->message) * 8;
            int16_t start_x = (ssd1306_width - message_width_pixels) / 2;
            if (start_x < 0) {
                start_x = 0;
            }
            ssd1306_draw_string(buffer, start_x, alert_state->line_y_pixel, alert_state->message);
        }
        render_on_display(buffer, area);
        return true;
    }
    return false;
}