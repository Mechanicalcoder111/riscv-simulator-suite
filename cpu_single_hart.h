/********************************************************************************************
CSCI 463 – Final Exam Version 3 – RISC-V Simulator – Fall 2025

Programmer:  Aasim Ghani

Purpose:
    Declares the cpu_single_hart class, which represents a CPU containing a single
    RV32I hart. This subclass of rv32i_hart provides a run() method that:
      - Initializes register x2 with the size of the simulated memory.
      - Repeatedly calls tick() to execute instructions.
      - Honors an optional execution limit.
      - Reports the halt reason (if any) and the total number of instructions
        executed.
********************************************************************************************/


#ifndef CPU_SINGLE_HART_H
#define CPU_SINGLE_HART_H


#include <cstdint>
#include "rv32i_hart.h"


class cpu_single_hart : public rv32i_hart
{
public:
    // Pass the memory reference up to the rv32i_hart base class.
    cpu_single_hart(memory &mem) : rv32i_hart(mem) {}


    // Run the hart until halted or the instruction limit is reached.
    void run(uint64_t exec_limit);
};


#endif

