# Elevator Project

## Prerequisites
- Ubuntu/Linux environment
- Clang compiler
- Make build system

## Installation
```bash
# Install required compiler
sudo apt-get update
sudo apt-get install clang
```

## Building
```bash
make ./elevator
```

## Running
```bash
./elevator
```

## Project Structure
```
skeleton_project/
├── source/
│   ├── driver/
│   │   └── elevio.c
│   └── main.c
├── build/
└── Makefile
```

## Compilation Flags
- Wall: Enable all compiler warnings
- g: Include debugging information
- std=gnu11: Use GNU C11 standard
- fsanitize=address: Enable AddressSanitizer for memory error detection

## Cleaning
```bash
make clean
```

## Rebuilding
```bash
make rebuild
```
