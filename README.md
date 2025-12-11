RISC-V Simulator Suite (RV32I)

A complete RISC-V RV32I simulator written in modern C++. This project implements the core components of a real RISC-V processor, including:

Memory module

Register file

Instruction decoder and disassembler

Single-hart CPU execution engine

Instruction-accurate execution flow

Tracing, debugging, and program loading utilities

The simulator executes binary programs one instruction at a time while maintaining correct architectural state.

Features

RV32I Instruction Set Support
Implements all RV32I base instructions:

Arithmetic and logical

Load and store

Branch and jump

Immediate and register variants

System instructions (ECALL, EBREAK)

CPU Execution Engine

Program counter updates

ALU processing

Memory access logic

Illegal instruction detection

Optional full instruction trace mode

Memory System

Byte, halfword, and word access

Safe bounds checking

Memory inspection utilities

Disassembler

Converts machine code into readable RV32I assembly

Register File

32 registers (x0–x31)

x0 permanently zero

Register dump utilities

Project Structure

cpu_single_hart.cpp / .h - CPU execution engine
rv32i_decode.cpp / .h - Instruction decoder and disassembler
rv32i_hart.cpp / .h - Instruction implementations
memory.cpp / .h - Memory model
registerfile.cpp / .h - Register file
hex.cpp / .h - Hex loader
main.cpp - Command-line interface

Building

Compile manually with g++:

g++ -std=c++17 -Wall -Wextra -o rv32i main.cpp cpu_single_hart.cpp rv32i_decode.cpp rv32i_hart.cpp memory.cpp registerfile.cpp hex.cpp

Or build using a Makefile:

make

Running

Run a program:
./rv32i program.hex

Enable instruction tracing:
./rv32i -t program.hex

Dump registers after execution:
./rv32i -l program.hex

Example Test Files

Common test programs include:
add.hex
branch.hex
memtest.hex

Example:
./rv32i -t add.hex

Purpose

This project demonstrates:

CPU architecture fundamentals

Instruction decoding

State transitions inside a CPU

Memory and register interactions

Binary program execution

Systems programming in C++

It provides a foundation for understanding how processors interpret and execute machine instructions.

Author

Aasim Ghani
Software Developer
Systems Programming • C++ • Computer Architecture
