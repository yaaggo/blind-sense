#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
// Macros

#define BUZZER_A_PIN 21
#define BUZZER_B_PIN 10

// Cabeçalhos

// Funçao para inicializa o Buzzer
void buzzer_init(uint8_t gpio);

// Funçao para liga o Buzzer em uma determinada frequência
void buzzer_turn_on(uint8_t gpio, uint16_t freq);

// Funçao para desliga o Buzzer
void buzzer_turn_off(uint8_t gpio);

// Funçao para ativa o Buzzer por um tempo determinado (em milissegundos)
void buzzer_beep(uint8_t gpio, uint16_t freq, uint16_t duration_ms);

#endif