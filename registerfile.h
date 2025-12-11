/********************************************************************************************
RISC-V Simulator

Programmer:  Aasim Ghani

Purpose:
    Declares the 'registerfile' class, which stores the 32 general-purpose
    registers for a single RISC-V hart. Provides methods to reset, read,
    write, and dump register values. Register x0 is hard-wired to 0.
********************************************************************************************/


#ifndef REGISTERFILE_H
#define REGISTERFILE_H


#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include "hex.h"


class registerfile : public hex
{
public:
    // Constructor: initialize all registers via reset()
    registerfile();


    // Initialize x0 = 0, x1..x31 = 0xf0f0f0f0
    void reset();


    // Write val into register r (ignored if r == 0)
    void set(uint32_t r, int32_t val);


    // Read value of register r (returns 0 if r == 0)
    int32_t get(uint32_t r) const;


    // Dump all registers, 8 per line, with optional header prefix
    void dump(const std::string &hdr) const;


private:
    std::vector<int32_t> regs;  // 32 registers x0..x31
};


#endif

