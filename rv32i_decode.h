/********************************************************************************************
CSCI 463 – PE1 – RV32I Disassembler / Memory Simulator – Fall 2024




Programmer:  Aasim Ghani
Section:     2
TA:          James Alexander Stephans
Date Due:    10/30/2024




Purpose:
    Declares the 'rv32i_decode' class, which performs static decoding of
    32-bit RV32I instruction words into human-readable assembly language.
    The class:
      - Defines opcode and formatting constants.
      - Extracts instruction fields from raw 32-bit values.
      - Renders assembly mnemonics and operands into formatted strings.
    All methods are static; this class is never instantiated and does not
    perform any I/O itself.




I certify that this is my own work, and where applicable an extension of the
starter code for the assignment/exam.
********************************************************************************************/


#ifndef RV32I_DECODE_H
#define RV32I_DECODE_H


#include <cstdint>
#include <string>
#include "hex.h"


class rv32i_decode : public hex
{
public:
    // RV32I register width and mnemonic formatting width.
    static constexpr uint32_t XLEN = 32;
    static constexpr int mnemonic_width = 8;


    // Opcode constants for supported instruction groups.
    static constexpr uint32_t opcode_lui      = 0b0110111;
    static constexpr uint32_t opcode_auipc    = 0b0010111;
    static constexpr uint32_t opcode_jal      = 0b1101111;
    static constexpr uint32_t opcode_jalr     = 0b1100111;
    static constexpr uint32_t opcode_btype    = 0b1100011;
    static constexpr uint32_t opcode_load     = 0b0000011;
    static constexpr uint32_t opcode_store    = 0b0100011;
    static constexpr uint32_t opcode_alu_imm  = 0b0010011;
    static constexpr uint32_t opcode_alu_reg  = 0b0110011;
    static constexpr uint32_t opcode_system   = 0b1110011;


    // Top-level decode: convert one instruction into a formatted string.
    static std::string decode(uint32_t addr, uint32_t insn);


    // Field extractors.
    static uint32_t get_opcode(uint32_t insn);
    static uint32_t get_rd(uint32_t insn);
    static uint32_t get_rs1(uint32_t insn);
    static uint32_t get_rs2(uint32_t insn);
    static uint32_t get_funct3(uint32_t insn);
    static uint32_t get_funct7(uint32_t insn);
    static int32_t  get_imm_i(uint32_t insn);
    static int32_t  get_imm_u(uint32_t insn);
    static int32_t  get_imm_b(uint32_t insn);
    static int32_t  get_imm_s(uint32_t insn);
    static int32_t  get_imm_j(uint32_t insn);


protected:
    // Render an illegal/unimplemented instruction.
    static std::string render_illegal_insn();


    // Small helpers for consistent formatting of mnemonics and operands.
    static std::string render_mnemonic(const std::string &mnemonic);
    static std::string render_reg(int r);
    static std::string render_base_disp(uint32_t rs1, int32_t imm);


    // Instruction-specific render helpers.
    static std::string render_lui(uint32_t insn);
    static std::string render_auipc(uint32_t insn);
    static std::string render_jal(uint32_t addr, uint32_t insn);
    static std::string render_jalr(uint32_t insn);


    static std::string render_btype(uint32_t addr, uint32_t insn, const std::string &mnemonic);
    static std::string render_itype_load(uint32_t insn, const std::string &mnemonic);
    static std::string render_stype(uint32_t insn, const std::string &mnemonic);
    static std::string render_itype_alu(uint32_t insn, const std::string &mnemonic, int32_t imm);
    static std::string render_rtype(uint32_t insn, const std::string &mnemonic);
    static std::string render_csrrx(uint32_t insn, const std::string &mnemonic);
    static std::string render_csrrxi(uint32_t insn, const std::string &mnemonic);
};


#endif

