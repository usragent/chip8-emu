#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

void chip8_init(chip8_t *chip8) {
    memset(chip8, 0, sizeof(chip8_t));
    memcpy(chip8->memory, fontset, FONTSET_SIZE);
    chip8->pc = PROGRAM_START;
    srand((unsigned int)time(NULL));
}

int chip8_load_rom(chip8_t *chip8, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return -1;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    
    if (size > MEMORY_SIZE - PROGRAM_START) {
        fclose(file);
        return -1;
    }
    
    fread(&chip8->memory[PROGRAM_START], 1, size, file);
    fclose(file);
    return 0;
}

static void execute_arithmetic(chip8_t *chip8, uint8_t x, uint8_t y, uint8_t op) {
    switch (op) {
        case 0x0:
            chip8->V[x] = chip8->V[y];
            break;
        case 0x1:
            chip8->V[x] |= chip8->V[y];
            break;
        case 0x2:
            chip8->V[x] &= chip8->V[y];
            break;
        case 0x3:
            chip8->V[x] ^= chip8->V[y];
            break;
        case 0x4: {
            uint16_t sum = chip8->V[x] + chip8->V[y];
            chip8->V[0xF] = sum > 0xFF ? 1 : 0;
            chip8->V[x] = sum & 0xFF;
            break;
        }
        case 0x5:
            chip8->V[0xF] = chip8->V[x] >= chip8->V[y] ? 1 : 0;
            chip8->V[x] -= chip8->V[y];
            break;
        case 0x6:
            chip8->V[0xF] = chip8->V[x] & 0x1;
            chip8->V[x] >>= 1;
            break;
        case 0x7:
            chip8->V[0xF] = chip8->V[y] >= chip8->V[x] ? 1 : 0;
            chip8->V[x] = chip8->V[y] - chip8->V[x];
            break;
        case 0xE:
            chip8->V[0xF] = (chip8->V[x] & 0x80) >> 7;
            chip8->V[x] <<= 1;
            break;
    }
}

static void execute_draw(chip8_t *chip8, uint8_t x, uint8_t y, uint8_t height) {
    uint8_t x_pos = chip8->V[x] % DISPLAY_WIDTH;
    uint8_t y_pos = chip8->V[y] % DISPLAY_HEIGHT;
    chip8->V[0xF] = 0;
    
    for (uint8_t row = 0; row < height; row++) {
        if (y_pos + row >= DISPLAY_HEIGHT) break;
        uint8_t sprite_byte = chip8->memory[chip8->I + row];
        
        for (uint8_t col = 0; col < 8; col++) {
            if (x_pos + col >= DISPLAY_WIDTH) break;
            
            if (sprite_byte & (0x80 >> col)) {
                uint8_t px = x_pos + col;
                uint8_t py = y_pos + row;
                uint32_t mask = 1U << px;
                
                if (chip8->display[py] & mask) {
                    chip8->V[0xF] = 1;
                }
                chip8->display[py] ^= mask;
            }
        }
    }
    chip8->draw_flag = true;
}

static void execute_misc(chip8_t *chip8, uint8_t x, uint8_t op) {
    switch (op) {
        case 0x07:
            chip8->V[x] = chip8->delay_timer;
            break;
        case 0x0A: {
            bool pressed = false;
            for (uint8_t i = 0; i < NUM_KEYS; i++) {
                if (chip8->keypad[i]) {
                    chip8->V[x] = i;
                    pressed = true;
                    break;
                }
            }
            if (!pressed) {
                chip8->pc -= 2;
            }
            break;
        }
        case 0x15:
            chip8->delay_timer = chip8->V[x];
            break;
        case 0x18:
            chip8->sound_timer = chip8->V[x];
            break;
        case 0x1E:
            chip8->I += chip8->V[x];
            break;
        case 0x29:
            chip8->I = (chip8->V[x] & 0xF) * 5;
            break;
        case 0x33:
            chip8->memory[chip8->I] = chip8->V[x] / 100;
            chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
            chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
            break;
        case 0x55:
            for (uint8_t i = 0; i <= x; i++) {
                chip8->memory[chip8->I + i] = chip8->V[i];
            }
            break;
        case 0x65:
            for (uint8_t i = 0; i <= x; i++) {
                chip8->V[i] = chip8->memory[chip8->I + i];
            }
            break;
    }
}

void chip8_cycle(chip8_t *chip8) {
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;
    uint8_t kk = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;
    
    chip8->pc += 2;
    
    switch (opcode & 0xF000) {
        case 0x0000:
            if (opcode == 0x00E0) {
                memset(chip8->display, 0, sizeof(chip8->display));
                chip8->draw_flag = true;
            } else if (opcode == 0x00EE) {
                chip8->sp--;
                chip8->pc = chip8->stack[chip8->sp];
            }
            break;
        
        case 0x1000:
            chip8->pc = nnn;
            break;
        
        case 0x2000:
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = nnn;
            break;
        
        case 0x3000:
            if (chip8->V[x] == kk) chip8->pc += 2;
            break;
        
        case 0x4000:
            if (chip8->V[x] != kk) chip8->pc += 2;
            break;
        
        case 0x5000:
            if (chip8->V[x] == chip8->V[y]) chip8->pc += 2;
            break;
        
        case 0x6000:
            chip8->V[x] = kk;
            break;
        
        case 0x7000:
            chip8->V[x] += kk;
            break;
        
        case 0x8000:
            execute_arithmetic(chip8, x, y, n);
            break;
        
        case 0x9000:
            if (chip8->V[x] != chip8->V[y]) chip8->pc += 2;
            break;
        
        case 0xA000:
            chip8->I = nnn;
            break;
        
        case 0xB000:
            chip8->pc = nnn + chip8->V[0];
            break;
        
        case 0xC000:
            chip8->V[x] = (rand() % 256) & kk;
            break;
        
        case 0xD000:
            execute_draw(chip8, x, y, n);
            break;
        
        case 0xE000:
            if (kk == 0x9E && chip8->keypad[chip8->V[x] & 0xF]) {
                chip8->pc += 2;
            } else if (kk == 0xA1 && !chip8->keypad[chip8->V[x] & 0xF]) {
                chip8->pc += 2;
            }
            break;
        
        case 0xF000:
            execute_misc(chip8, x, kk);
            break;
    }
}

void chip8_update_timers(chip8_t *chip8) {
    if (chip8->delay_timer > 0) {
        chip8->delay_timer--;
    }
    if (chip8->sound_timer > 0) {
        chip8->sound_timer--;
    }
}

void chip8_set_key(chip8_t *chip8, uint8_t key, uint8_t state) {
    if (key < NUM_KEYS) {
        chip8->keypad[key] = state;
    }
}

bool chip8_get_pixel(const chip8_t *chip8, uint8_t x, uint8_t y) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return false;
    }
    return (chip8->display[y] & (1U << x)) != 0;
}
