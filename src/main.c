#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "include/joystick.h"
#include "include/display.h"
#include "include/led.h"
#include "include/matrix.h"
#include "include/buzzer.h"

#include "menu.h"

#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define DEBOUNCE_DELAY 100

#define DEBUG(var) printf("%s: %d\n", #var, var);

void button_callback(uint gpio, uint32_t events);
void button_init(uint8_t pin);


static volatile uint32_t last_a_interrupt_time = 0;
static volatile uint32_t last_b_interrupt_time = 0;
static volatile uint32_t last_joystick_interrupt_time = 0;

volatile bool button_a_state = 0;
volatile bool button_a_previous_state = 0;

display dp;

void init_all() {
    button_init(BUTTON_A_PIN);
    button_init(BUTTON_B_PIN);
    button_init(JOYSTICK_BUTTON_PIN);

    buzzer_init(BUZZER_A_PIN);

    display_init(&dp);

}

int main() {
    stdio_init_all();

    init_all();

    menu_start_scene(&dp);
    display_update(&dp);
    
    while(true) {

        sleep_ms(100);

    }
    
    return 0;
}

void button_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // verifica qual botão acionou a interrupção e trata o debounce
    if (gpio == BUTTON_A_PIN) {
        if (current_time - last_a_interrupt_time > DEBOUNCE_DELAY) {
            last_a_interrupt_time = current_time;

            if (events & GPIO_IRQ_EDGE_FALL) {
                button_a_state = !button_a_state;
                button_a_previous_state = !button_a_state;
            }
        }
    } else if (gpio == BUTTON_B_PIN) { // botao extra para entrar em modo bootsel
        if (current_time - last_b_interrupt_time > DEBOUNCE_DELAY) {
            last_b_interrupt_time = current_time;

            if (events & GPIO_IRQ_EDGE_FALL) {   
                reset_usb_boot(0, 0);
            }
        }
    } else if (gpio == JOYSTICK_BUTTON_PIN) { 
        if (current_time - last_joystick_interrupt_time > DEBOUNCE_DELAY) {
            last_joystick_interrupt_time = current_time;

            if (events & GPIO_IRQ_EDGE_FALL) {   

            }
        }
    }
}

// inicializa o botão e configura interrupções
void button_init(uint8_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);

    // habilita interrupções para bordas de descida
    gpio_set_irq_enabled_with_callback(
        pin,
        GPIO_IRQ_EDGE_FALL,
        true,
        &button_callback
    );
}