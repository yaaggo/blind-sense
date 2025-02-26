#include "buzzer.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"

// Funções

// Funçao para inicializa o Buzzer
void buzzer_init(uint8_t gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM); // Configura o GPIO para PWM
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_enabled(slice_num, false); // Desabilita o PWM inicialmente
}

// Funçao para liga o Buzzer em uma determinada frequência
void buzzer_turn_on(uint8_t gpio, uint16_t freq) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    uint clock_div = 4; // Divisor do clock
    uint sys_clock = clock_get_hz(clk_sys); // Frequência do sistema
    uint wrap = sys_clock / (clock_div * freq) - 1; // Calcula o valor de "wrap" para a frequência desejada

    pwm_set_clkdiv(slice_num, clock_div); // Define o divisor do clock para a frequência
    pwm_set_wrap(slice_num, wrap); // Define o valor de "wrap" para a frequência
    pwm_set_gpio_level(gpio, wrap / 2); // Ciclo de trabalho de 50% (som contínuo)
    pwm_set_enabled(slice_num, true); // Habilita o PWM
}

// Funçao para desliga o Buzzer
void buzzer_turn_off(uint8_t gpio) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_enabled(slice_num, false); // Desabilita o PWM (desliga o buzzer)
}

// Funçao para ativa o Buzzer por um tempo determinado (em milissegundos)
void buzzer_beep(uint8_t gpio, uint16_t freq, uint16_t duration_ms) {
    buzzer_turn_on(gpio, freq); // Liga o buzzer com a frequência especificada
    sleep_ms(duration_ms); // Espera o tempo determinado
    buzzer_turn_off(gpio); // Desliga o buzzer após o tempo
}