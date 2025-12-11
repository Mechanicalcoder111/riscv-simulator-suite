# RISC-V Simulator Suite (RV32I)

A complete RISC-V RV32I simulator written in modern C++, implementing:

- Memory module  
- Register file  
- Instruction decoder & disassembler  
- Single-hart CPU emulator  
- Instruction-accurate RV32I execution engine  
- Tracing, debugging output, and program loaders

This project replicates the core behavior of a real RISC-V processor, executing binary programs one instruction at a time while maintaining correct architectural state.

---

## Features

### RV32I Instruction Set Support
Implements all base RV32I instructions:

- Arithmetic & logical operations  
- Load/store  
- Branch & jump  
- Immediate + register variants  
- System instructions (`ECALL`, `EBREAK`)

### CPU Execution Engine
- Program counter management  
- ALU operations  
- Memory access  
- Illegal-instruction handling  
- Optional trace mode showing each executed instruction  

### Memory System
- Byte / halfword / word reads and writes  
- Safe address access  
- Memory dumping utilities  

### Disassembler
Converts machine code into human-readable RV32I assembly that matches standard encoding formats.

### Register File
- 32 registers (x0–x31)  
- x0 hardwired to zero  
- Register state dump utilities  

---

## Project Structure

```
cpu_single_hart.cpp / .h   # CPU execution engine  
rv32i_decode.cpp / .h      # Instruction decoder + disassembler  
rv32i_hart.cpp / .h        # Instruction implementations  
memory.cpp / .h            # Memory model  
registerfile.cpp / .h      # Register file  
hex.cpp / .h               # Hex loader  
main.cpp                   # Command-line interface
```

---

## Building the Simulator

Compile using g++:

```bash
g++ -std=c++17 -Wall -Wextra -o rv32i \
    main.cpp cpu_single_hart.cpp rv32i_decode.cpp \
    rv32i_hart.cpp memory.cpp registerfile.cpp hex.cpp
```

Or using your Makefile:

```bash
make
```

---

## Running the Simulator

Run a hex program:

```bash
./rv32i prog.hex
```

Enable tracing:

```bash
./rv32i -t prog.hex
```

Dump registers at the end:

```bash
./rv32i -l prog.hex
```

---

## Example Test Files

Common test programs include:

- `add.hex`  
- `branch.hex`  
- `memtest.hex`

Example usage:

```bash
./rv32i -t add.hex
```

---

## Purpose

This project demonstrates:

- CPU architecture fundamentals  
- Instruction decoding  
- State transitions in a CPU pipeline  
- Memory/register interactions  
- Binary program execution  
- Systems programming in C++

It serves as a foundation for understanding how real processors interpret and execute machine code.

---

## Author

**Aasim Ghani**  
Software Developer  
Systems Programming • C++ • Computer Architecture
