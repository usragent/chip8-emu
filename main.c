#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <rom_file>\n", argv[0]);
        return 1;
    }
    
    chip8_t chip8;
    chip8_init(&chip8);
    
    if (chip8_load_rom(&chip8, argv[1]) != 0) {
        fprintf(stderr, "Failed to load ROM: %s\n", argv[1]);
        return 1;
    }
    
    printf("ROM loaded successfully\n");
    
    for (int i = 0; i < 10; i++) {
        chip8_cycle(&chip8);
        chip8_update_timers(&chip8);
        
        if (chip8.draw_flag) {
            printf("Draw flag set\n");
            chip8.draw_flag = false;
        }
    }
    
    return 0;
}
