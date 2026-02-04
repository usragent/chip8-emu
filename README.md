# CHIP-8 Emulator

A CHIP-8 virtual machine implementation in C99.

## Building

```bash
make
```

## Usage

```bash
./chip8 <rom_file>
```

## Architecture

- **Memory**: 4KB RAM (0x000-0xFFF)
- **Display**: 64x32 monochrome
- **Registers**: 16 8-bit (V0-VF)
- **Stack**: 16 levels
- **Timers**: Delay and sound (60Hz)
## Files

- `chip8.h` / `chip8.c` - Core emulator
- `main.c` - Entry point
- `Makefile` - Build configuration
