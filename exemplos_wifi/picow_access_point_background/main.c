/**
 * Projeto: Servidor HTTP com controle de Alerta (Buzzer/Display/LED) via AP - Pico W
 */

#include "pico/stdlib.h"
#include "hardware/i2c.h"  // Para i2c_inst_t
#include "hardware/gpio.h"
#include <string.h>        // Para memset, strncmp, strchr, sscanf, snprintf

// Includes para o Display OLED
#include "inc/display/display_app.h"
#include "inc/display/ssd1306_i2c.h" // Para ssd1306_buffer_length, struct render_area
#include "inc/display/oled_messages.h" // Para BlinkingAlertState

// Includes para Rede e Servidor HTTP
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "dhcpserver.h"
#include "dnsserver.h"

// Include para o Buzzer
#include "inc/buzzer.h"

// --- Definições do Servidor HTTP e Aplicação ---
#define TCP_PORT 80
#define DEBUG_printf printf
#define POLL_TIME_S 5
#define HTTP_GET "GET "
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
#define LED_TEST_BODY "<html><head><title>PicoW Control</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head><body><h1>Controle de Alerta PicoW</h1><p>Estado do Alerta (Buzzer/Display/LED): <strong>%s</strong></p><p><a href=\"/?led=1\" role=\"button\">ATIVAR ALERTA</a></p><p><a href=\"/?led=0\" role=\"button\">DESATIVAR ALERTA</a></p></body></html>"
#define LED_PARAM "led=%d"
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://%s/\n\n"

// --- Pinos GPIO ---
#define BUZZER_PIN 21

// --- Variáveis Estáticas para o Display OLED ---
static uint8_t g_oled_buffer[ssd1306_buffer_length];
static struct render_area g_oled_render_area;
static BlinkingAlertState g_oled_alert_state;
static i2c_inst_t* g_i2c_port_display = i2c1;

// --- Estruturas para o Servidor TCP ---
typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
    char ap_name[32];
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[128];
    char result[512];
    int header_len;
    int result_len;
    ip_addr_t *gw;
    char *ap_name_ptr;
} TCP_CONNECT_STATE_T;


// --- Callbacks e Funções do Servidor TCP ---
static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        if (con_state && con_state->pcb != client_pcb) {
            // Não fazer nada com con_state se não pertencer a este pcb
        } else if (con_state) {
             assert(con_state->pcb == client_pcb);
        }

        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);

        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("tcp_close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

static void tcp_server_close(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (!con_state) return ERR_OK;

    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

static int server_handle_request(const char *params, char *result, size_t max_result_len) {
    int http_param_led_value = -1;

    if (params) {
        sscanf(params, LED_PARAM, &http_param_led_value);
    }

    if (http_param_led_value == 1) {
        DEBUG_printf("HTTP Request: ATIVAR Alerta\n");
        g_play_music_flag = true;
        display_application_activate_alert();
        
    } else if (http_param_led_value == 0) {
        DEBUG_printf("HTTP Request: DESATIVAR Alerta\n");
        g_play_music_flag = false;
        display_application_deactivate_alert();
        
    }

    if (g_play_music_flag) {
        return snprintf(result, max_result_len, LED_TEST_BODY, "ATIVO (EVACUAR)");
    } else {
        return snprintf(result, max_result_len, LED_TEST_BODY, "INATIVO (SEGURO)");
    }
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err_in) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;

    if (!p) {
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    assert(con_state && con_state->pcb == pcb);

    if (p->tot_len > 0) {
        char request_buffer[128];
        memset(request_buffer, 0, sizeof(request_buffer));
        pbuf_copy_partial(p, request_buffer, p->tot_len > sizeof(request_buffer) - 1 ? sizeof(request_buffer) - 1 : p->tot_len, 0);

        if (strncmp(HTTP_GET, request_buffer, strlen(HTTP_GET)) == 0) {
            char *request_path = request_buffer + strlen(HTTP_GET);
            char *params_start = strchr(request_path, '?');
            char *http_version_start = strstr(request_path, " HTTP/");

            if (http_version_start) {
                *http_version_start = 0;
            }

            char *actual_params = NULL;
            if (params_start) {
                *params_start = 0;
                actual_params = params_start + 1;
            }

            con_state->result_len = server_handle_request(actual_params, con_state->result, sizeof(con_state->result));

            if (con_state->result_len > 0) {
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
                                                 200, con_state->result_len);
            } else {
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_REDIRECT,
                                                 con_state->ap_name_ptr ? con_state->ap_name_ptr : ipaddr_ntoa(con_state->gw));
            }

            if (con_state->header_len > sizeof(con_state->headers) - 1) {
                DEBUG_printf("Header buffer overflow %d\n", con_state->header_len);
                return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
            }

            con_state->sent_len = 0;
            err_t err_write = tcp_write(pcb, con_state->headers, con_state->header_len, TCP_WRITE_FLAG_COPY);
            if (err_write != ERR_OK) {
                DEBUG_printf("tcp_write failed for headers: %d\n", err_write);
                return tcp_close_client_connection(con_state, pcb, err_write);
            }

            if (con_state->result_len > 0) {
                err_write = tcp_write(pcb, con_state->result, con_state->result_len, TCP_WRITE_FLAG_COPY);
                if (err_write != ERR_OK) {
                    DEBUG_printf("tcp_write failed for body: %d\n", err_write);
                    return tcp_close_client_connection(con_state, pcb, err_write);
                }
            }
        }
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    return tcp_close_client_connection(con_state, pcb, ERR_OK);
}

static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_server_err: %d on pcb=%p con_state=%p\n", err, con_state ? (void*)con_state->pcb : NULL, (void*)con_state);
        if (con_state) {
             tcp_close_client_connection(con_state, con_state->pcb, err);
        }
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("Failure in accept\n");
        return ERR_VAL;
    }

    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        DEBUG_printf("Failed to allocate connect state\n");
        tcp_close(client_pcb);
        return ERR_MEM;
    }
    con_state->pcb = client_pcb;
    con_state->gw = &state->gw;
    con_state->ap_name_ptr = state->ap_name;

    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

static bool tcp_server_open(void *arg, const char *ap_name_param) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    strncpy(state->ap_name, ap_name_param, sizeof(state->ap_name) -1);
    state->ap_name[sizeof(state->ap_name) -1] = '\0';

    DEBUG_printf("Starting server on port %d\n", TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("Failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err) {
        DEBUG_printf("Failed to bind to port %d, error %d\n", TCP_PORT, err);
        tcp_close(pcb);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("Failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    printf("Servidor HTTP iniciado. Conecte-se a Wi-Fi '%s' e acesse http://%s\n",
           ap_name_param, ip4addr_ntoa(&state->gw));
    printf("(Pressione 'd' no serial para desabilitar o Access Point)\n");
    return true;
}

void key_pressed_func(void *param) {
    if (!param) return;
    TCP_SERVER_T *state = (TCP_SERVER_T*)param;
    int key = getchar_timeout_us(0);
    if (key == 'd' || key == 'D') {
        DEBUG_printf("Tecla 'd' pressionada, desabilitando AP...\n");
        cyw43_arch_lwip_begin();
        cyw43_arch_disable_ap_mode();
        cyw43_arch_lwip_end();
        state->complete = true;
    }
}

int main() {
    stdio_init_all();

    buzzer_init(BUZZER_PIN);
    g_play_music_flag = false;

   
    if (!display_application_init(g_oled_buffer, &g_oled_render_area, &g_oled_alert_state, g_i2c_port_display)) {
        DEBUG_printf("ERRO: Falha ao inicializar a aplicacao do display!\n");
    } else {
        DEBUG_printf("Aplicacao do display OLED inicializada.\n");
    }

    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("ERRO: Falha ao alocar estado do servidor TCP!\n");
        return 1;
    }

    if (cyw43_arch_init()) {
        DEBUG_printf("ERRO: Falha ao inicializar cyw43_arch!\n");
        free(state);
        return 1;
    }
    DEBUG_printf("cyw43_arch inicializado.\n");

    stdio_set_chars_available_callback(key_pressed_func, state);

    const char *ap_name = "PicoW_Alerta";
    const char *password = "12345678";

    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);
    DEBUG_printf("Modo Access Point '%s' habilitado.\n", ap_name);

    ip4_addr_t mask;
    ip4_addr_set_u32(&state->gw, PP_HTONL(CYW43_DEFAULT_IP_AP_ADDRESS));
    ip4_addr_set_u32(&mask, PP_HTONL(CYW43_DEFAULT_IP_MASK));

    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);
    DEBUG_printf("Servidor DHCP iniciado.\n");

    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);
    DEBUG_printf("Servidor DNS iniciado.\n");

    if (!tcp_server_open(state, ap_name)) {
        DEBUG_printf("ERRO: Falha ao abrir o servidor TCP!\n");
        dns_server_deinit(&dns_server);
        dhcp_server_deinit(&dhcp_server);
        cyw43_arch_deinit();
        free(state);
        return 1;
    }

    state->complete = false;
    DEBUG_printf("Loop principal iniciado.\n");

    while(!state->complete) {
        if (g_play_music_flag) {
            buzzer_play(BUZZER_PIN);
        }

        display_application_process(); // Chamada para atualizar o display

        // ***************************************************************
        // MODIFICAÇÃO AQUI: Diminuindo o tempo de espera no loop principal
        // ***************************************************************
#if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(10)); // Era 100ms
#else
        sleep_ms(10); // Era 100ms
#endif
        // ***************************************************************
    }

    DEBUG_printf("Finalizando aplicacao...\n");
    tcp_server_close(state);
    dns_server_deinit(&dns_server);
    dhcp_server_deinit(&dhcp_server);
    cyw43_arch_deinit();
    free(state);
    printf("Aplicacao finalizada.\n");
    return 0;
}