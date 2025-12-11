/********************************************************************************************
RISC-V Simulator

Programmer:  Aasim Ghani

Purpose:
    Implements the 'hex' utility class used to convert integer values into formatted
    hexadecimal strings. These helpers are used throughout the RV32I disassembler and
    memory simulator to render addresses, instruction words, and immediate fields in
    a consistent hex format (with or without a 0x prefix and fixed widths).
********************************************************************************************/


#include <sstream>
#include <iostream>
#include <string>
#include <cstdint>
#include <cctype>
#include <iomanip>
#include "hex.h"
using namespace std;


/***************************************************************
Function: hex::to_hex8


Use:      Converts an 8-bit value into a 2-character lowercase
          hexadecimal string with leading zeros.


Arguments:
    i - Unsigned 8-bit value to be formatted.


Returns:
    A std::string containing exactly two hex digits (0-9, a-f).
***************************************************************/
std::string hex::to_hex8(uint8_t i)
{
    std::ostringstream os;
    os << std::hex << std::setfill('0') << std::setw(2)
       << static_cast<uint16_t>(i);
    return os.str();
}


/***************************************************************
Function: hex::to_hex32


Use:      Converts a 32-bit value into an 8-character lowercase
          hexadecimal string with leading zeros.


Arguments:
    i - Unsigned 32-bit value to be formatted.


Returns:
    A std::string containing exactly eight hex digits.
***************************************************************/
std::string hex::to_hex32(uint32_t i)
{
    std::ostringstream os;
    os << std::hex << std::setfill('0') << std::setw(8) << i;
    return os.str();
}


/***************************************************************
Function: hex::to_hex0x32


Use:      Converts a 32-bit value into an 8-character lowercase
          hexadecimal string with a leading "0x" prefix.


Arguments:
    i - Unsigned 32-bit value to be formatted.


Returns:
    A std::string beginning with "0x" followed by eight hex digits.
***************************************************************/
std::string hex::to_hex0x32(uint32_t i)
{
    return std::string("0x")+to_hex32(i);
}


/***************************************************************
Function: hex::to_hex0x20


Use:      Converts a 32-bit value into a hexadecimal string that
          represents only the least-significant 20 bits, with a
          leading "0x" prefix.


Arguments:
    i - Unsigned 32-bit value; only the 20 LSBs are used.


Returns:
    A std::string beginning with "0x" followed by exactly 5 hex
    digits representing bits [19:0] of the input.
***************************************************************/
std::string hex::to_hex0x20(uint32_t i)
{
   std::ostringstream os;
   os << std::hex << std::setfill('0') << std::setw(5)
      << (i & 0x000fffff);
   return "0x" + os.str();
}


/***************************************************************
Function: hex::to_hex0x12


Use:      Converts a 32-bit value into a hexadecimal string that
          represents only the least-significant 12 bits, with a
          leading "0x" prefix.


Arguments:
    i - Unsigned 32-bit value; only the 12 LSBs are used.


Returns:
    A std::string beginning with "0x" followed by exactly 3 hex
    digits representing bits [11:0] of the input.
***************************************************************/
std::string hex::to_hex0x12(uint32_t i)
{
   std::ostringstream os;
   os << std::hex << std::setfill('0') << std::setw(3)
      << (i & 0x00000fff);
   return "0x" + os.str();
}



