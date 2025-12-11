/********************************************************************************************
RISC-V Simulator

Programmer:  Aasim Ghani

Purpose:
    Implements the 'memory' class, which simulates a byte-addressable memory array
    for the RV32I disassembler. It provides methods to:
      - Allocate and track the simulated memory size.
      - Read and write 8-, 16-, and 32-bit values (little-endian).
      - Read sign-extended values (8-, 16-, and 32-bit forms).
      - Load a binary file into memory.
      - Dump the entire memory contents in both hex and ASCII formats.
    This class is used by main() and rv32i_decode to fetch instructions and
    report warnings for out-of-range memory accesses.
********************************************************************************************/


#include "memory.h"
#include "hex.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cctype>


/***************************************************************
Function: memory::memory


Use:      Constructor that allocates the simulated memory and
          initializes all bytes to 0xa5.


Arguments:
    s - Requested size of the memory in bytes. The value is
        rounded up to the next multiple of 16.


Returns:
    Nothing (constructor).
***************************************************************/
memory::memory(uint32_t s)
{
    s = (s + 15) & 0xfffffff0;          // round size up to multiple of 16
    mem = std::vector<uint8_t>(s, 0xa5);
}


/***************************************************************
Function: memory::~memory


Use:      Destructor for the memory object. The std::vector
          automatically releases its storage.


Arguments:
    None.


Returns:
    Nothing.
***************************************************************/
memory::~memory()
{
}


/***************************************************************
Function: memory::check_illegal


Use:      Checks whether the given address is outside the valid
          range of the simulated memory. If it is out of range, a
          warning is printed to std::cerr.


Arguments:
    addr - Byte address to be checked.


Returns:
    true  if the address is illegal (out of range).
    false if the address is within the valid memory range.
***************************************************************/
bool memory::check_illegal(uint32_t addr) const
{
    if(addr >= mem.size())
    {
        std::cerr << "WARNING: Address out of range: " << hex::to_hex0x32(addr) << std::endl;
        return true;
    }
    return false;
}


/***************************************************************
Function: memory::check_illegal


Use:      Checks whether the given address is outside the valid
          range of the simulated memory. If it is out of range, a
          warning is printed to std::cerr.


Arguments:
    addr - Byte address to be checked.


Returns:
    true  if the address is illegal (out of range).
    false if the address is within the valid memory range.
***************************************************************/
uint32_t memory::get_size() const
{
    return static_cast<uint32_t>(mem.size());
}


/***************************************************************
Function: memory::get8


Use:      Reads one byte from the simulated memory.


Arguments:
    addr - Byte address of the value to read.


Returns:
    The 8-bit value at the given address. If the address is
    illegal, returns 0 and prints a warning.
***************************************************************/
uint8_t memory::get8(uint32_t addr) const
{
    if (!check_illegal(addr))
    {  
        return mem[addr];
    }
    else
    {
        return 0;
    }
}


/***************************************************************
Function: memory::get16


Use:      Reads a 16-bit little-endian value from the simulated
          memory by combining two consecutive bytes.


Arguments:
    addr - Address of the low-order byte of the 16-bit value.


Returns:
    The 16-bit value composed from bytes at addr and addr+1.
    If an address is out of range, the read will silently use
    0 from get8() for that byte.
***************************************************************/
uint16_t memory::get16(uint32_t addr) const
{
    uint16_t first = get8(addr);
    uint16_t second = get8(addr + 1);
    second <<= 8;
    return first + second;
}


/***************************************************************
Function: memory::get32


Use:      Reads a 32-bit little-endian value from the simulated
          memory by combining two consecutive 16-bit values.


Arguments:
    addr - Address of the low-order byte of the 32-bit value.


Returns:
    The 32-bit value composed from four bytes starting at addr.
***************************************************************/
uint32_t memory::get32(uint32_t addr) const
{
    uint32_t first = get16(addr);
    uint32_t second = get16(addr + 2);
    second <<= 16;
    return first | second;
}


/***************************************************************
Function: memory::get8_sx


Use:      Reads an 8-bit value from memory and sign-extends it
          into a 32-bit signed integer.


Arguments:
    addr - Address of the byte to read.


Returns:
    A 32-bit signed integer with the sign bit taken from bit 7
    of the original 8-bit value.
***************************************************************/
int32_t memory::get8_sx(uint32_t addr) const
{
    uint8_t insb = get8(addr);
    uint8_t msb = insb >> 7;
    uint32_t bitmask = 0xffffff00;
    return (msb == 1) ? (int32_t)insb | bitmask : (int32_t)insb;
}


/***************************************************************
Function: memory::get16_sx


Use:      Reads a 16-bit value from memory and sign-extends it
          into a 32-bit signed integer.


Arguments:
    addr - Address of the low-order byte of the 16-bit value.


Returns:
    A 32-bit signed integer with the sign bit taken from bit 15
    of the original 16-bit value.
***************************************************************/
int32_t memory::get16_sx(uint32_t addr) const
{
    uint16_t insb = get16(addr);
    uint16_t msb = insb >> 15;
    uint32_t bitmask = 0xffff0000;
    return (msb == 1) ? (int32_t)insb | bitmask : (int32_t)insb;
}


/***************************************************************
Function: memory::get32_sx


Use:      Reads a 32-bit value from memory and interprets it as
          a signed 32-bit integer.


Arguments:
    addr - Address of the low-order byte of the 32-bit value.


Returns:
    The 32-bit value read from memory, reinterpreted as int32_t.
***************************************************************/
int32_t memory::get32_sx(uint32_t addr) const
{
    return get32(addr);
}


/***************************************************************
Function: memory::set8


Use:      Writes one byte into the simulated memory.


Arguments:
    addr - Address where the byte will be stored.
    val  - 8-bit value to write.


Returns:
    Nothing. If addr is illegal, the write is ignored and a
    warning is printed by check_illegal().
***************************************************************/
void memory::set8(uint32_t addr, uint8_t val)
{
    if(!check_illegal(addr))
    {
        mem[addr] = val;
    }
}


/***************************************************************
Function: memory::set16


Use:      Writes a 16-bit value into memory in little-endian
          order (low byte first).


Arguments:
    addr - Address of the low-order byte to write.
    val  - 16-bit value to store.


Returns:
    Nothing. Writes are performed using set8(), which protects
    against illegal addresses.
***************************************************************/
void memory::set16(uint32_t addr, uint16_t val)
{
    uint8_t low_byte = val & 0xff;
    uint8_t high_byte = (val >> 8) & 0xff;
    set8(addr, low_byte);
    set8(addr + 1, high_byte);
}


/***************************************************************
Function: memory::set32


Use:      Writes a 32-bit value into memory in little-endian
          order by storing two 16-bit halves.


Arguments:
    addr - Address of the low-order byte to write.
    val  - 32-bit value to store.


Returns:
    Nothing.
***************************************************************/
void memory::set32(uint32_t addr, uint32_t val)
{
    uint16_t low_word = val & 0xffff;
    uint16_t high_word = (val >> 16) & 0xffff;
    set16(addr, low_word);
    set16(addr + 2, high_word);
}


/***************************************************************
Function: memory::dump


Use:      Prints a formatted dump of the entire simulated memory.
          Each line shows:
            - The starting address (hex).
            - 16 bytes of data in hex (grouped).
            - The same 16 bytes as printable ASCII characters,
              with non-printable bytes rendered as '.'.


Arguments:
    None.


Returns:
    Nothing. Output goes to std::cout.
***************************************************************/
void memory::dump() const
{
    for(uint32_t i = 0; i < mem.size(); i+=16)
    {
        std::cout << to_hex32(i) << ": ";


        // Print 16 bytes of hex data.
        for(uint32_t j = 0; j < 16; j++)
        {
            std::cout << to_hex8(get8(i + j)) << " ";
            if (j == 7)
                std::cout << " ";
        }


        // Print ASCII representation between asterisks.        
        std::cout << "*";
        for(uint32_t j = 0; j < 16; j++)
        {
            uint8_t ch = get8(i + j);
            ch = isprint(ch) ? ch : '.';
            std::cout << ch;
        }
        std::cout << "*" << std::endl;
    }
}


/***************************************************************
Function: memory::load_file


Use:      Loads a binary file into the simulated memory starting
          at address 0. Reads bytes until EOF or until the memory
          is full.


Arguments:
    fname - Name of the binary file to open and read.


Returns:
    true  if the file was opened successfully and all bytes that
          fit into memory were loaded.
    false if the file could not be opened or if the program did
          not fit (in which case a message "Program too big." is
          printed and loading stops).
***************************************************************/
bool memory::load_file(const std::string &fname)
{
   
    std::ifstream infile(fname, std::ios::in|std::ios::binary);


    if(!infile.is_open())
    {
        std::cerr << "Can't open file '" << fname
                  << "' for reading." << std::endl;
        return false;
    }


    uint8_t i;
    infile >> std::noskipws;
   
    for (uint32_t addr = 0; infile >> i; ++addr)
    {
        if(check_illegal(addr))
        {
            std::cerr << "Program too big." << std::endl;
            infile.close();
            return false;
        }


        set8(addr, i);
    }


    infile.close();
    return true;
}

