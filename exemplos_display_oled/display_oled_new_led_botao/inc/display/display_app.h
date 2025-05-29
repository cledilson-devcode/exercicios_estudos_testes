// inc/display_app.h
#ifndef DISPLAY_APP_H
#define DISPLAY_APP_H

// Substitui o include de oled_blinking_alert.h por oled_messages.h
#include "inc/display/oled_messages.h"     // Para BlinkingAlertState e funções de alerta
#include "inc/display/ssd1306_i2c.h"       // Para struct render_area (ou o .h apropriado, ex: "inc/ssd1306.h")
#include "hardware/i2c.h"          // Para i2c_inst_t

// Inicializa todos os componentes da aplicação do display.
bool display_application_init(uint8_t *buffer_ptr, struct render_area *area_ptr,
                              BlinkingAlertState *alert_state_ptr, i2c_inst_t *i2c_port_ptr);

// Processa um ciclo da lógica do display.
void display_application_process(void);

// Função para alternar o estado do alerta, chamada pelo main.c
void display_application_toggle_alert_state(void);

#endif // DISPLAY_APP_H