/********************************************************************************************
CSCI 463 - Final Exam Version 3 - RISC-V Simulator - Fall 2025




Programmer:  Aasim Ghani
Section:     2
TA:          James Alexander Stephans
Date Due:    10/30/2025




Purpose:
    Main driver for the RV32I simulator.


    This program:
      - Parses the command-line for:
            [-d] [-i] [-r] [-z] [-l exec-limit] [-m hex-mem-size] infile
      - Constructs a 'memory' object of the requested size and loads the
        binary file into it.
      - Optionally disassembles the entire memory before simulation (-d).
      - Constructs a cpu_single_hart, configures its flags, and runs it
        with an optional instruction-count limit (-l).
      - Optionally dumps the final hart state and memory (-z).




I certify that this is my own work, and where applicable an extension of the
starter code for the assignment/exam.
********************************************************************************************/


#include <iostream>
#include <cstdlib>
#include <sstream>
#include <unistd.h>


#include "memory.h"
#include "hex.h"
#include "rv32i_decode.h"
#include "cpu_single_hart.h"


using namespace std;


/***************************************************************
Function: usage


Use:      Prints the required usage message and exits.


Arguments:
    progname - argv[0] (program name) for the error message.


Returns:
    Never returns; calls exit(1).
***************************************************************/
static void usage(const char *progname)
{
    cerr << "Usage: rv32i [-d] [-i] [-r] [-z] [-l exec-limit] "
         << "[-m hex-mem-size] infile" << endl;
    cerr << "  -d show disassembly before program execution" << endl;
    cerr << "  -i show instruction printing during execution" << endl;
    cerr << "  -l maximum number of instructions to exec" << endl;
    cerr << "  -m specify memory size (default = 0x100)" << endl;
    cerr << "  -r show register printing during execution" << endl;
    cerr << "  -z show a dump of the regs & memory after simulation" << endl;
    exit(1);
}


/***************************************************************
Function: disassemble


Use:      Walks through memory and disassembles each 32-bit word.


Arguments:
    mem - Reference to a 'memory' object containing the loaded
          binary program.


Returns:
    Nothing. Output is sent to std::cout.
***************************************************************/
static void disassemble(const memory &mem)
{
    for (uint32_t addr = 0; addr < mem.get_size(); addr += 4)
    {
        uint32_t insn = mem.get32(addr);


        cout << hex::to_hex32(addr) << ": "
             << hex::to_hex32(insn) << "  "
             << rv32i_decode::decode(addr, insn)
             << endl;
    }
}


/***************************************************************
Function: main


Use:      Entry point for the RV32I simulator.


Arguments:
    argc - Number of command-line arguments.
    argv - Argument vector.


Returns:
    0 on success. Exits via usage() on errors.
***************************************************************/
int main(int argc, char **argv)
{
    // Default options
    uint32_t memory_limit = 0x100;
    uint64_t exec_limit   = 0;     // 0 = no limit


    bool dflag = false;            // -d: disassemble before execution
    bool iflag = false;            // -i: show instructions
    bool rflag = false;            // -r: show registers
    bool zflag = false;            // -z: dump regs & memory after


    // ------------------------------------------------------------
    // Parse command-line options using getopt.
    // ------------------------------------------------------------
    int opt;
    while ((opt = getopt(argc, argv, "dirzl:m:")) != -1)
    {
        switch (opt)
        {
        case 'd':
            dflag = true;
            break;


        case 'i':
            iflag = true;
            break;


        case 'r':
            rflag = true;
            break;


        case 'z':
            zflag = true;
            break;


        case 'l':
        {
            // execution limit is decimal
            std::istringstream iss(optarg);
            iss >> exec_limit;
            break;
        }


        case 'm':
        {
            // memory size is hex
            std::istringstream iss(optarg);
            iss >> std::hex >> memory_limit;
            break;
        }


        default:
            // invalid option (like -X)
            cerr << argv[0] << ": invalid option -- '" << (char)optopt << "'" << endl;
            usage(argv[0]);
        }
    }


    // After options, we must have exactly one infile argument.
    if (optind >= argc)
    {
        usage(argv[0]);
    }


    const char *infile = argv[optind];


    // ------------------------------------------------------------
    // Create simulated memory and load the input file.
    // ------------------------------------------------------------
    memory mem(memory_limit);


    if (!mem.load_file(infile))
    {
        // load_file already printed an error message
        return 1;
    }


    // ------------------------------------------------------------
    // Optional disassembly before simulation (-d).
    // ------------------------------------------------------------
    if (dflag)
    {
        disassemble(mem);
    }


    // ------------------------------------------------------------
    // Create CPU, configure it, and run the simulation.
    // ------------------------------------------------------------
    cpu_single_hart cpu(mem);
    cpu.reset();


    cpu.set_show_instructions(iflag);
    cpu.set_show_registers(rflag);


    cpu.run(exec_limit);


    // ------------------------------------------------------------
    // Optional final dumps (-z).
    // ------------------------------------------------------------
    if (zflag)
    {
        cpu.dump();   // dump registers + pc
        mem.dump();   // dump memory
    }


    return 0;
}

