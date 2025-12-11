/********************************************************************************************
RISC-V Simulator

Programmer:  Aasim Ghani

Purpose:
    Declares the 'memory' class, which simulates a contiguous byte-addressable
    memory for RV32I programs. It supports reading and writing 8-, 16-, and
    32-bit values, reading sign-extended values, loading binary files, and
    dumping the entire memory contents for debugging and disassembly.
********************************************************************************************/


#ifndef MEMORY_H
#define MEMORY_H


#include <cstdint>
#include <string>
#include <vector>
#include "hex.h"


/***************************************************************
Class: memory


Use:   Models a simple byte-addressable memory system used by the
       RV32I disassembler and simulator. Provides methods to:
         - Check for illegal addresses.
         - Load, store, and sign-extend values of various sizes.
         - Load a binary program into memory.
         - Dump the entire memory in a human-readable format.


Data:
       mem - std::vector<uint8_t> holding the raw memory bytes.
***************************************************************/
class memory : public hex
{
public:


    // Construct memory with size rounded up to a multiple of 16 bytes.
    explicit memory(uint32_t s);


    // Destructor (vector automatically frees resources).
    ~memory();


    // Check if an address is out of range, printing a warning if so.
    bool check_illegal(uint32_t addr) const;


    // Return total size of the simulated memory in bytes.
    uint32_t get_size() const;


    // Read 8 bits from memory.
    uint8_t get8(uint32_t addr) const;


    // Read 16 bits from memory (little-endian).
    uint16_t get16(uint32_t addr) const;


    // Read 32 bits from memory (little-endian).
    uint32_t get32(uint32_t addr) const;


    // Read 8 bits and sign-extend to 32 bits.
    int32_t get8_sx(uint32_t addr) const;


    // Read 16 bits and sign-extend to 32 bits.
    int32_t get16_sx(uint32_t addr) const;


    // Read 32 bits and interpret as signed.
    int32_t get32_sx(uint32_t addr) const;


    // Write 8 bits into memory.
    void set8(uint32_t addr, uint8_t val);


    // Write 16 bits into memory (little-endian).
    void set16(uint32_t addr, uint16_t val);


    // Write 32 bits into memory (little-endian).
    void set32(uint32_t addr, uint32_t val);


    // Dump the entire contents of memory in hex and ASCII.
    void dump() const;


    // Load a binary file into memory starting at address 0.
    bool load_file(const std::string &fname);


private:
    // Underlying storage for the simulated memory.
    std::vector<uint8_t> mem;
};


#endif

