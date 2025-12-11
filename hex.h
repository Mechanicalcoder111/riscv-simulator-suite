/********************************************************************************************
RISC-V Simulator

Programmer:  Aasim Ghani

Purpose:
    Declares the 'hex' utility class, which provides a collection of static helper
    functions for formatting 8-bit and 32-bit integer values as hexadecimal strings.
    These helpers centralize the formatting rules (width, leading zeros, and "0x"
    prefixes) so that all printed hex values in the program are consistent.
********************************************************************************************/


#ifndef HEX_H
#define HEX_H


#include <string>
#include <cstdint>


/***************************************************************
Class: hex


Use:   Utility class providing static helper functions to convert
       integers into formatted hexadecimal strings. It is never
       instantiated; all methods are static and used by other
       classes (memory, rv32i_decode, main) for consistent hex
       output formatting.


Data:
       (No data members – purely a utility class.)
***************************************************************/
class hex
{
public:
    // Convert an 8-bit value to a 2-digit hex string (no "0x" prefix).
    static std::string to_hex8(uint8_t i);


    // Convert a 32-bit value to an 8-digit hex string (no "0x" prefix).
    static std::string to_hex32(uint32_t i);


    // Convert a 32-bit value to an 8-digit hex string with a "0x" prefix.
    static std::string to_hex0x32(uint32_t i);


    // Convert the least-significant 20 bits of a 32-bit value to "0x" + 5 digits.
    static std::string to_hex0x20(uint32_t i);
   
    // Convert the least-significant 12 bits of a 32-bit value to "0x" + 3 digits.
    static std::string to_hex0x12(uint32_t i);
};


#endif

