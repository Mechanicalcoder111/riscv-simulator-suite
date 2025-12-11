/********************************************************************************************
RISC-V Simulator

Programmer:  Aasim Ghani

Purpose:
    Implements the cpu_single_hart class run() method. This method:
      - Sets register x2 to the size of the simulated memory so that programs
        can determine the memory size.
      - Executes instructions by repeatedly calling tick(), either:
          * until the hart is halted (exec_limit == 0), or
          * until the hart is halted or the instruction-count limit is reached.
      - If the hart halts, prints the halt reason.
      - Always prints the total number of instructions executed.
********************************************************************************************/


#include "cpu_single_hart.h"
#include <iostream>


using std::cout;
using std::endl;


void cpu_single_hart::run(uint64_t exec_limit)
{
    // Per assignment: x2 contains the memory size (in bytes) before execution.
    // mem and regs are protected members of rv32i_hart.
    regs.set(2, static_cast<int32_t>(mem.get_size()));


    if (exec_limit == 0)
    {
        // No limit: run until the hart halts.
        while (!is_halted())
        {
            tick();
        }
    }
    else
    {
        // Limited run: stop when halted OR when limit reached.
        while (!is_halted() && get_insn_counter() < exec_limit)
        {
            tick();
        }
    }


    // If we halted, report the reason (matches assignment text).
    if (is_halted())
    {
        cout << "Execution terminated. Reason: "
             << get_halt_reason() << endl;
    }


    // Always report how many instructions were executed.
    cout << get_insn_counter() << " instructions executed" << endl;
}

