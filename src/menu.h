#ifndef MENU_H
#define MENU_H

#include "include/display.h"
#include "pico/stdlib.h"
#include <stdint.h>

#define ARROWS_Y 30

enum menu_stage {
    MENU_INITIAL,
    MENU_INSTRUCTION,
    MENU_IN_EXECUTION
};

extern const uint8_t seta_normal[120];
void menu_initial_scene(display *dp);
void menu_instruction_scene(display *dp);
void menu_in_execution_scene(display *dp);

#endif