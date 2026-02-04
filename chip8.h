#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#include <stdbool.h>

#define MEMORY_SIZE 4096
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define STACK_SIZE 16
#define NUM_REGISTERS 16
#define NUM_KEYS 16
#define FONTSET_SIZE 80
#define PROGRAM_START 0x200

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[NUM_REGISTERS];
    uint16_t I;
    uint16_t pc;
    uint8_t sp;
    uint16_t stack[STACK_SIZE];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t keypad[NUM_KEYS];
    uint32_t display[DISPLAY_HEIGHT];
    bool draw_flag;
} chip8_t;

void chip8_init(chip8_t *chip8);
int chip8_load_rom(chip8_t *chip8, const char *filename);
void chip8_cycle(chip8_t *chip8);
void chip8_update_timers(chip8_t *chip8);
void chip8_set_key(chip8_t *chip8, uint8_t key, uint8_t state);
bool chip8_get_pixel(const chip8_t *chip8, uint8_t x, uint8_t y);

#endif
