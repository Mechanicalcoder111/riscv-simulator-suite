/********************************************************************************************
CSCI 463 – Final Exam Version 3 – RISC-V Simulator – Fall 2025


Programmer:  Aasim Ghani
Section:     2
TA:          James Alexander Stephans
Date Due:    10/30/2025


Purpose:
    Declares the rv32i_hart class, which represents the execution unit (hart)
    of an RV32I CPU. This class:


      - Owns the general-purpose register file.
      - Holds the program counter (pc) and status flags.
      - Fetches and executes instructions from the 'memory' object.
      - Provides tracing/dumping options controlled by flags.


    Instruction decoding is inherited from rv32i_decode; this class adds the
    dynamic execution behavior.


I certify that this is my own work, and where applicable an extension of the
starter code for the assignment/exam.
********************************************************************************************/


#ifndef RV32I_HART_H
#define RV32I_HART_H


#include <cstdint>
#include <string>
#include <ostream>


#include "rv32i_decode.h"
#include "registerfile.h"
#include "memory.h"


class rv32i_hart : public rv32i_decode
{
public:
    rv32i_hart(memory &m) : mem(m) {}


    // Control flags
    void set_show_instructions(bool b) { show_instructions = b; }
    void set_show_registers(bool b)    { show_registers    = b; }


    // Status
    bool is_halted() const                 { return halt; }
    const std::string &get_halt_reason() const { return halt_reason; }
    uint64_t get_insn_counter() const      { return insn_counter; }


    // Hart ID (for future extensions)
    void set_mhartid(int i)                { mhartid = i; }


    // Execution interface
    void tick(const std::string &hdr = "");
    void dump(const std::string &hdr = "") const;
    void reset();


protected:
    registerfile regs;    // General-purpose registers
    memory &mem;          // Reference to simulated memory


private:
    static constexpr int instruction_width = 35;


    void exec(uint32_t insn, std::ostream *pos);
    void exec_illegal_insn(uint32_t insn, std::ostream *pos);


    void exec_lui(uint32_t insn, std::ostream *pos);
    void exec_auipc(uint32_t insn, std::ostream *pos);
    void exec_jal(uint32_t insn, std::ostream *pos);
    void exec_jalr(uint32_t insn, std::ostream *pos);
    void exec_alu_imm(uint32_t insn, std::ostream *pos);
    void exec_alu_reg(uint32_t insn, std::ostream *pos);
    void exec_load(uint32_t insn, std::ostream *pos);
    void exec_store(uint32_t insn, std::ostream *pos);
    void exec_branch(uint32_t insn, std::ostream *pos);


    void exec_csrrx(uint32_t insn, std::ostream *pos, const std::string &mnemonic);
    void exec_csrrxi(uint32_t insn, std::ostream *pos, const std::string &mnemonic);
    void exec_ecall(uint32_t insn, std::ostream *pos);
    void exec_ebreak(uint32_t insn, std::ostream *pos);


    // Hart state
    bool halt         = false;
    std::string halt_reason = "none";
    bool show_instructions  = false;
    bool show_registers     = false;


    uint64_t insn_counter   = 0;
    uint32_t pc             = 0;
    uint32_t mhartid        = 0;


    // Simple CSR storage (4K CSRs is plenty for this assignment)
    uint32_t csr[4096] = {0};
};


#endif

