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

int constrain(int valor, int minimo, int maximo) {
    if (valor < minimo) {
        return minimo;
    } else if (valor > maximo) {
        return maximo;
    } else {
        return valor;
    }
}

int16_t x_position = 0;
int16_t y_position = 0;

uint8_t initial_index = 0;
uint8_t direction_index = 0;

bool selected = false;
bool matrix_is_on = false;

int8_t matrix_x, matrix_y;

enum menu_stage menu_actual_stage = MENU_INITIAL;

uint16_t buzzer_direction[] = {500, 1000, 1500, 2000};

int main() {
    stdio_init_all();
    init_all();

    while(true) {
        matrix_x = 0, matrix_y = 0;
        x_position = joystick_read(JOYSTICK_X_PIN, 20, 400);
        y_position = joystick_read(JOYSTICK_Y_PIN, 20, 400);
        DEBUG(y_position);
        DEBUG(x_position);
        if(menu_actual_stage != MENU_IN_EXECUTION && matrix_is_on) {
            matrix_clear();
            matrix_update();
            matrix_is_on = false;
        }

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
                bool change = false;
                matrix_is_on = true;
                matrix_clear();
                display_clear(&dp);

                int center = 2;
                int max_offset = 2;
                
                // Definir pontos de ativação baseados na faixa do joystick
                int step1 = 40, step2 = 100, step3 = 160;

                matrix_set_led_xy(center, center, COLOR_RGB(0, 3, 0)); // Ponto central (indivíduo)

                int dx = 0, dy = 0;

                if (x_position || y_position) {
                    change = true;

                    // Inverter a lógica: bordas -> centro
                    if (x_position > step3) {
                        dx = -max_offset;
                    } else if (x_position > step2) {
                        dx = -1;
                    } else if (x_position < -step3) {
                        dx = max_offset;
                    } else if (x_position < -step2) {
                        dx = 1;
                    }

                    if (y_position > step3) {
                        dy = -max_offset;
                    } else if (y_position > step2) {
                        dy = -1;
                    } else if (y_position < -step3) {
                        dy = max_offset;
                    } else if (y_position < -step2) {
                        dy = 1;
                    }

                    // O efeito agora parte das bordas e vem ao centro
                    matrix_x = center + dx;
                    matrix_y = center + dy;
                }

                if (change) {
                    uint abs_x = abs(x_position);
                    uint abs_y = abs(y_position);
                    matrix_set_led_xy(matrix_x, matrix_y, COLOR_RGB(3, 0, 0));
                    if(abs_x > abs_y) {
                        if (matrix_x == 0) {
                            buzzer_beep(BUZZER_B_PIN, buzzer_direction[3], 300);
                        } else if(matrix_x == 4) {
                            buzzer_beep(BUZZER_B_PIN, buzzer_direction[1], 300);
                        }
                    } else {
                        if (matrix_y == 0) {
                            buzzer_beep(BUZZER_B_PIN, buzzer_direction[0], 300);
                        } else if(matrix_y == 4) {
                            buzzer_beep(BUZZER_B_PIN, buzzer_direction[2], 300);
                        }
                    }
                }

                
                matrix_update();

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
                matrix_clear();
                matrix_update();
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