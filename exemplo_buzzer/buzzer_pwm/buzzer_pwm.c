/**
 * Exemplo de acionamento do buzzer utilizando sinal PWM no GPIO 21 da Raspberry Pico / BitDogLab
 * 06/12/2024
 */

#include <stdio.h>
#include "inc/buzzer.h" 
#include "pico/stdlib.h"

// Configuração do pino do buzzer
#define BUZZER_PIN 21



int main() {
    stdio_init_all(); // Inicializa stdio (para USB serial, etc.)

    // Inicializa o buzzer usando a função do nosso módulo
    buzzer_music_init(BUZZER_PIN);

    while (1) {
        // Toca a música Star Wars usando a função do nosso módulo
        // buzzer_music_play_star_wars(BUZZER_PIN);

        g_play_music_flag = !g_play_music_flag; // Alterna o estado da flag

        if (g_play_music_flag) {
            buzzer_music_play_star_wars(BUZZER_PIN);
            // Se a música terminou e g_play_music_flag ainda é true, ela vai
            // ser chamada de novo na próxima iteração do while(1), tocando continuamente.
            // Se você quer que toque apenas uma vez por pressionamento de botão,
            // você deve setar g_play_music_flag = false; após a chamada acima OU
            // dentro de buzzer_music_play_star_wars no final da música.
            // Para o comportamento "parar quando desejar", este loop contínuo é o correto.
        } else {
            // Se a música não deve tocar, podemos colocar um pequeno delay
            // para não sobrecarregar o processador apenas lendo o botão.
            sleep_ms(20); // Verifica o botão aproximadamente 50 vezes por segundo.
        }
        
        // Adiciona um delay maior entre as repetições da música, se desejar
        // sleep_ms(2000); 
    }

    return 0; // Nunca será alcançado no Pico em um loop infinito
}