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

    joystick_init(JOYSTICK_X_PIN, JOYSTICK_Y_PIN);
    
    buzzer_init(BUZZER_B_PIN);

    matrix_init(MATRIX_LED_PIN);

    display_init(&dp);
}

int16_t x_position = 0;
int16_t y_position = 0;

uint8_t initial_index = 0;
uint8_t direction_index = 0;

bool selected = false;

enum menu_stage menu_actual_stage = MENU_INITIAL;

uint16_t buzzer_direction[] = {500, 1000, 1500, 2000};

int main() {
    stdio_init_all();
    init_all();

    while(true) {

        x_position = joystick_read(JOYSTICK_X_PIN, 20, 400);
        y_position = joystick_read(JOYSTICK_Y_PIN, 20, 400);
        DEBUG(y_position);
        DEBUG(x_position);
        switch(menu_actual_stage) {

            case MENU_INITIAL:
                if(y_position > 60) {
                    initial_index = initial_index ? 0 : 1;
                    buzzer_beep(BUZZER_B_PIN, 100, 200);
                } else if(y_position < -60) {
                    initial_index = initial_index ? 0 : 1;
                    buzzer_beep(BUZZER_B_PIN, 200, 200);
                }

                menu_initial_scene(initial_index, &dp);

            break;

            case MENU_INSTRUCTION:
                if(x_position > 60) {
                    direction_index = direction_index >= 3 ? 0 : direction_index + 1;
                    buzzer_beep(BUZZER_B_PIN, 100, 200);
                } else if(x_position < -60) {
                    direction_index = direction_index <= 0 ? 3 : direction_index - 1;
                    buzzer_beep(BUZZER_B_PIN, 100, 200);
                }

                if(selected) {
                    menu_instruction_scene(direction_index, selected,&dp);
                    display_update(&dp);
                    buzzer_beep(BUZZER_B_PIN, buzzer_direction[direction_index], 400);
                    selected = false;
                }

                menu_instruction_scene(direction_index, selected,&dp);
            break;

            case MENU_IN_EXECUTION:
                
            break;
        }

        display_update(&dp);
        
        sleep_ms(30);
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
                if(menu_actual_stage == MENU_INITIAL) {
                    if(initial_index == 0) {
                        menu_actual_stage = MENU_IN_EXECUTION;
                    } else {
                        menu_actual_stage = MENU_INSTRUCTION;
                    }
                } else if (menu_actual_stage == MENU_INSTRUCTION) {
                    selected = true;
                }
            }
        }
    } else if (gpio == BUTTON_B_PIN) { // botao extra para entrar em modo bootsel
        if (current_time - last_b_interrupt_time > DEBOUNCE_DELAY) {
            last_b_interrupt_time = current_time;

            if (events & GPIO_IRQ_EDGE_FALL) {   
                menu_actual_stage = MENU_INITIAL;
            }
        }
    } else if (gpio == JOYSTICK_BUTTON_PIN) { 
        if (current_time - last_joystick_interrupt_time > DEBOUNCE_DELAY) {
            last_joystick_interrupt_time = current_time;

            if (events & GPIO_IRQ_EDGE_FALL) {
                buzzer_turn_off(BUZZER_B_PIN);
                display_shutdown(&dp);
                reset_usb_boot(0, 0);
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