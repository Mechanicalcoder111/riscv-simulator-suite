/********************************************************************************************
RISC-V Simulator

Programmer:  Aasim Ghani

Purpose:
    Implements the 'registerfile' class that models the 32 general-purpose
    registers in a RISC-V hart. Register x0 always reads as 0 and ignores writes.
********************************************************************************************/


#include "registerfile.h"


/***************************************************************
Function: registerfile::registerfile


Use:
    Constructor. Initializes the internal vector to hold 32
    registers and calls reset() to set their initial values.


Arguments:
    None.


Returns:
    Nothing.
***************************************************************/
registerfile::registerfile()
    : regs(32)
{
    reset();
}


/***************************************************************
Function: registerfile::reset


Use:
    Initialize register x0 to 0 and x1..x31 to 0xf0f0f0f0.


Arguments:
    None.


Returns:
    Nothing.
***************************************************************/
void registerfile::reset()
{
    regs[0] = 0;
    for (std::size_t i = 1; i < regs.size(); ++i)
    {
        regs[i] = 0xf0f0f0f0;
    }
}


/***************************************************************
Function: registerfile::set


Use:
    Assign register r the given value. Writes to x0 are ignored.


Arguments:
    r   - Register index (0..31).
    val - Value to write.


Returns:
    Nothing.
***************************************************************/
void registerfile::set(uint32_t r, int32_t val)
{
    if (r == 0 || r >= regs.size())
        return;


    regs[r] = val;
}


/***************************************************************
Function: registerfile::get


Use:
    Return the current value of register r. Reads from x0
    always return 0.


Arguments:
    r - Register index (0..31).


Returns:
    int32_t value of the register.
***************************************************************/
int32_t registerfile::get(uint32_t r) const
{
    if (r == 0 || r >= regs.size())
        return 0;


    return regs[r];
}


/***************************************************************
Function: registerfile::dump


Use:
    Print the contents of all 32 registers in four rows of 8
    registers each. The hdr string is printed at the start of
    each line (may be empty).


    Example with hdr == "":
      x0 00000000 f0f0f0f0 ...


    Example with hdr == "HEADER-":
      HEADER- x0 00000000 ...


Arguments:
    hdr - String printed at the beginning of each line.


Returns:
    Nothing. Output goes to std::cout.
***************************************************************/
void registerfile::dump(const std::string &hdr) const
{
    for (int base = 0; base < 32; base += 8)
    {
        // Line header: hdr + register label (x0, x8, x16, x24)
        std::cout << hdr << 'x' << base;


        // Align single-digit indices ("x0 ", "x8 ") like the handout
        if (base < 10)
            std::cout << ' ';


        // Print 8 registers on this line
        for (int i = 0; i < 8; ++i)
        {
            int idx = base + i;
            std::cout << ' ' << hex::to_hex32(static_cast<uint32_t>(regs[idx]));
        }


        std::cout << std::endl;
    }
}

