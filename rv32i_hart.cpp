/********************************************************************************************
RISC-V Simulator

Programmer:  Aasim Ghani

Purpose:
    Implements the rv32i_hart class, which simulates execution of RV32I
    instructions on a single hart. This file supports:


      - reset(): initialize pc, regs, counters, flags, and CSRs.
      - dump(): print the GP-registers and pc.
      - tick(): execute exactly one instruction (with optional tracing).
      - exec(): dispatch instructions by opcode/funct3/funct7.
      - exec_* helpers for:
           LUI, AUIPC, JAL, JALR,
           branches, loads, stores,
           ALU-imm, ALU-reg,
           CSR ops, ECALL, EBREAK,
           and illegal instructions.
********************************************************************************************/


#include "rv32i_hart.h"
#include "hex.h"


#include <iostream>
#include <iomanip>


using std::cout;
using std::endl;
using std::ostream;
using std::string;


/***************************************************************
Function: rv32i_hart::reset
***************************************************************/
void rv32i_hart::reset()
{
    pc           = 0;
    insn_counter = 0;
    halt         = false;
    halt_reason  = "none";
    mhartid      = 0;


    regs.reset();


    // Clear CSRs
    for (auto &c : csr)
        c = 0;
}


void rv32i_hart::dump(const std::string &hdr) const
{
    (void)hdr; // not used for this assignment


    auto print_row = [&](const char *label, int start_reg)
    {
        // label already includes correct spacing (e.g., " x0 ", "x16 ")
        std::cout << label;


        // Print 8 registers, hex 8 digits each.
        // One space between values, but two spaces after the 4th value.
        // IMPORTANT: no trailing space after the last value.
        for (int i = 0; i < 8; ++i)
        {
            int r = start_reg + i;
            uint32_t val = static_cast<uint32_t>(regs.get(r));
            std::cout << hex::to_hex32(val);


            if (i < 7)           // only print spaces between values
            {
                if (i == 3)
                    std::cout << "  ";
                else
                    std::cout << " ";
            }
        }
        std::cout << '\n';
    };


    print_row(" x0 ",  0);
    print_row(" x8 ",  8);
    print_row("x16 ", 16);
    print_row("x24 ", 24);


    std::cout << " pc " << hex::to_hex32(pc) << '\n';
}
/***************************************************************
Function: rv32i_hart::tick
***************************************************************/
void rv32i_hart::tick(const std::string &hdr)
{
    if (halt)
        return;


    if (show_registers)
        dump(hdr);


    // PC alignment check
    if (pc & 0x3)
    {
        halt = true;
        halt_reason = "PC alignment error";
        return;
    }


    // Count this instruction
    insn_counter++;


    // Fetch instruction from memory
    uint32_t insn = mem.get32(pc);


    if (show_instructions)
    {
        cout << hdr
             << hex::to_hex32(pc) << ": "
             << hex::to_hex32(insn) << "  ";


        exec(insn, &cout);
        cout << endl;
    }
    else
    {
        exec(insn, nullptr);
    }
}


/***************************************************************
Function: rv32i_hart::exec
Use:   Dispatch one instruction by opcode.
***************************************************************/
void rv32i_hart::exec(uint32_t insn, std::ostream *pos)
{
    uint32_t opcode = get_opcode(insn);


    switch (opcode)
    {
    case opcode_lui:
        exec_lui(insn, pos);
        return;


    case opcode_auipc:
        exec_auipc(insn, pos);
        return;


    case opcode_jal:
        exec_jal(insn, pos);
        return;


    case opcode_jalr:
        exec_jalr(insn, pos);
        return;


    case opcode_alu_imm:
        exec_alu_imm(insn, pos);
        return;


    case opcode_alu_reg:
        exec_alu_reg(insn, pos);
        return;


    case opcode_load:
        exec_load(insn, pos);
        return;


    case opcode_store:
        exec_store(insn, pos);
        return;


    case opcode_btype:
        exec_branch(insn, pos);
        return;


    case opcode_system:
    {
        uint32_t f3 = get_funct3(insn);


        switch (f3)
        {
        case 0b000: // ecall / ebreak
            if (insn == 0x00000073)        // ecall
            {
                exec_ecall(insn, pos);
                return;
            }
            else if (insn == 0x00100073)   // ebreak
            {
                exec_ebreak(insn, pos);
                return;
            }
            else
            {
                exec_illegal_insn(insn, pos);
                return;
            }


        case 0b001: // csrrw
            exec_csrrx(insn, pos, "csrrw");
            return;


        case 0b010: // csrrs
            exec_csrrx(insn, pos, "csrrs");
            return;


        case 0b011: // csrrc
            exec_csrrx(insn, pos, "csrrc");
            return;


        case 0b101: // csrrwi
            exec_csrrxi(insn, pos, "csrrwi");
            return;


        case 0b110: // csrrsi
            exec_csrrxi(insn, pos, "csrrsi");
            return;


        case 0b111: // csrrci
            exec_csrrxi(insn, pos, "csrrci");
            return;


        default:
            exec_illegal_insn(insn, pos);
            return;
        }
    }


    default:
        exec_illegal_insn(insn, pos);
        return;
    }
}


/***************************************************************
Function: rv32i_hart::exec_illegal_insn
***************************************************************/
void rv32i_hart::exec_illegal_insn(uint32_t insn, std::ostream *pos)
{
    (void)insn; // not used, but kept for interface symmetry


    if (pos)
        *pos << render_illegal_insn();


    halt = true;
    halt_reason = "Illegal instruction";
}


/***************************************************************
Function: rv32i_hart::exec_lui


Use:   LUI: rd = imm_u; pc += 4
***************************************************************/
void rv32i_hart::exec_lui(uint32_t insn, std::ostream *pos)
{
    uint32_t rd  = get_rd(insn);
    int32_t  imm = get_imm_u(insn);      // already upper-imm << 12
    int32_t  val = imm;

    if (pos)
    {
        std::string s = render_lui(insn);
        *pos << std::setw(instruction_width)
             << std::setfill(' ')
             << std::left
             << s;

        // Comment must look like:  // x4 = 0xabcde000
        *pos << "// " << render_reg(rd)
             << " = " << hex::to_hex0x32(val);
    }

    regs.set(rd, val);
    pc += 4;
}

/***************************************************************
Function: rv32i_hart::exec_auipc


Use:   AUIPC: rd = pc + imm_u; pc += 4
***************************************************************/
void rv32i_hart::exec_auipc(uint32_t insn, std::ostream *pos)
{
    uint32_t rd   = get_rd(insn);
    int32_t  imm  = get_imm_u(insn);      // upper-imm << 12
    uint32_t old_pc = pc;
    int32_t  val  = int32_t(old_pc + imm);

    if (pos)
    {
        std::string s = render_auipc(insn);
        *pos << std::setw(instruction_width)
             << std::setfill(' ')
             << std::left
             << s;

        // Comment must look like:
        // // x4 = 0x00000004 + 0xabcde000 = 0xabcde004
        *pos << "// " << render_reg(rd)
             << " = " << hex::to_hex0x32(old_pc)
             << " + " << hex::to_hex0x32(imm)
             << " = " << hex::to_hex0x32(val);
    }

    regs.set(rd, val);
    pc += 4;
}

/***************************************************************
Function: rv32i_hart::exec_jal


Use:   JAL: rd = pc + 4; pc = pc + imm_j
***************************************************************/
void rv32i_hart::exec_jal(uint32_t insn, std::ostream *pos)
{
    uint32_t rd   = get_rd(insn);
    int32_t  imm  = get_imm_j(insn);


    uint32_t pc_before = pc;
    uint32_t target    = pc_before + imm;
    int32_t  retaddr   = static_cast<int32_t>(pc_before + 4);


    if (pos)
    {
        std::string s = render_jal(pc_before, insn);
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;
        *pos << "// " << render_reg(rd)
             << " = " << hex::to_hex0x32(retaddr)
             << ",  pc = " << hex::to_hex0x32(target);
    }


    regs.set(rd, retaddr);
    pc = target;
}


/***************************************************************
Function: rv32i_hart::exec_jalr


Use:   JALR: t = pc + 4; pc = (rs1 + imm_i) & ~1; rd = t
***************************************************************/
void rv32i_hart::exec_jalr(uint32_t insn, std::ostream *pos)
{
    uint32_t rd   = get_rd(insn);
    uint32_t rs1  = get_rs1(insn);
    int32_t  imm  = get_imm_i(insn);


    uint32_t pc_before = pc;
    uint32_t rs1_val   = static_cast<uint32_t>(regs.get(rs1));
    uint32_t target    = (rs1_val + imm) & ~uint32_t(1);
    int32_t  retaddr   = static_cast<int32_t>(pc_before + 4);


    if (pos)
    {
        std::string s = render_jalr(insn);
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;
        *pos << "// " << render_reg(rd)
             << " = " << hex::to_hex0x32(retaddr)
             << ",  pc = " << hex::to_hex0x32(target);
    }


    regs.set(rd, retaddr);
    pc = target;
}


/***************************************************************
Function: rv32i_hart::exec_alu_imm


Use:   I-type ALU: addi, slti, sltiu, xori, ori, andi,
       slli, srli, srai.
***************************************************************/
void rv32i_hart::exec_alu_imm(uint32_t insn, std::ostream *pos)
{
    uint32_t rd   = get_rd(insn);
    uint32_t rs1  = get_rs1(insn);
    uint32_t f3   = get_funct3(insn);
    uint32_t f7   = get_funct7(insn);


    int32_t  imm  = get_imm_i(insn);
    int32_t  rs1_val = regs.get(rs1);
    int32_t  result  = 0;


    std::string mnemonic;
    int32_t shown_imm = imm;


    switch (f3)
    {
    case 0b000: // addi
        mnemonic = "addi";
        result   = rs1_val + imm;
        break;


    case 0b010: // slti
        mnemonic = "slti";
        result   = (rs1_val < imm) ? 1 : 0;
        break;


    case 0b011: // sltiu
        mnemonic = "sltiu";
        result   = (static_cast<uint32_t>(rs1_val)
                    < static_cast<uint32_t>(imm)) ? 1 : 0;
        break;


    case 0b100: // xori
        mnemonic = "xori";
        result   = rs1_val ^ imm;
        break;


    case 0b110: // ori
        mnemonic = "ori";
        result   = rs1_val | imm;
        break;


    case 0b111: // andi
        mnemonic = "andi";
        result   = rs1_val & imm;
        break;


    case 0b001: // slli
        if (f7 != 0b0000000)
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        mnemonic   = "slli";
        shown_imm  = imm & 0x1f;
        result     = static_cast<int32_t>(
                       static_cast<uint32_t>(rs1_val) << shown_imm);
        break;


    case 0b101: // srli / srai
        shown_imm = imm & 0x1f;
        if (f7 == 0b0000000)
        {
            mnemonic = "srli";
            result   = static_cast<int32_t>(
                         static_cast<uint32_t>(rs1_val) >> shown_imm);
        }
        else if (f7 == 0b0100000)
        {
            mnemonic = "srai";
            result   = rs1_val >> shown_imm;
        }
        else
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        break;


    default:
        exec_illegal_insn(insn, pos);
        return;
    }


    if (pos)
    {
        std::string s = render_itype_alu(insn, mnemonic, shown_imm);
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;
        *pos << "// " << render_reg(rd)
             << " = " << hex::to_hex0x32(result);
    }


    regs.set(rd, result);
    pc += 4;
}


/***************************************************************
Function: rv32i_hart::exec_alu_reg


Use:   R-type ALU: add, sub, sll, slt, sltu,
       xor, srl, sra, or, and.
***************************************************************/
void rv32i_hart::exec_alu_reg(uint32_t insn, std::ostream *pos)
{
    uint32_t rd  = get_rd(insn);
    uint32_t rs1 = get_rs1(insn);
    uint32_t rs2 = get_rs2(insn);
    uint32_t f3  = get_funct3(insn);
    uint32_t f7  = get_funct7(insn);


    int32_t rs1_val = regs.get(rs1);
    int32_t rs2_val = regs.get(rs2);
    int32_t result  = 0;


    std::string mnemonic;


    switch (f3)
    {
    case 0b000: // add / sub
        if (f7 == 0b0000000)
        {
            mnemonic = "add";
            result   = rs1_val + rs2_val;
        }
        else if (f7 == 0b0100000)
        {
            mnemonic = "sub";
            result   = rs1_val - rs2_val;
        }
        else
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        break;


    case 0b001: // sll
        if (f7 != 0b0000000)
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        mnemonic = "sll";
        result   = static_cast<int32_t>(
                     static_cast<uint32_t>(rs1_val)
                     << (rs2_val & 0x1f));
        break;


    case 0b010: // slt
        if (f7 != 0b0000000)
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        mnemonic = "slt";
        result   = (rs1_val < rs2_val) ? 1 : 0;
        break;


    case 0b011: // sltu
        if (f7 != 0b0000000)
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        mnemonic = "sltu";
        result   = (static_cast<uint32_t>(rs1_val)
                    < static_cast<uint32_t>(rs2_val)) ? 1 : 0;
        break;


    case 0b100: // xor
        if (f7 != 0b0000000)
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        mnemonic = "xor";
        result   = rs1_val ^ rs2_val;
        break;


    case 0b101: // srl / sra
        if (f7 == 0b0000000)
        {
            mnemonic = "srl";
            result   = static_cast<int32_t>(
                         static_cast<uint32_t>(rs1_val)
                         >> (rs2_val & 0x1f));
        }
        else if (f7 == 0b0100000)
        {
            mnemonic = "sra";
            result   = rs1_val >> (rs2_val & 0x1f);
        }
        else
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        break;


    case 0b110: // or
        if (f7 != 0b0000000)
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        mnemonic = "or";
        result   = rs1_val | rs2_val;
        break;


    case 0b111: // and
        if (f7 != 0b0000000)
        {
            exec_illegal_insn(insn, pos);
            return;
        }
        mnemonic = "and";
        result   = rs1_val & rs2_val;
        break;


    default:
        exec_illegal_insn(insn, pos);
        return;
    }


    if (pos)
    {
        std::string s = render_rtype(insn, mnemonic);
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;
        *pos << "// " << render_reg(rd)
             << " = " << hex::to_hex0x32(result);
    }


    regs.set(rd, result);
    pc += 4;
}


/***************************************************************
Function: rv32i_hart::exec_load


Use:   Loads: lb, lh, lw, lbu, lhu.
***************************************************************/
void rv32i_hart::exec_load(uint32_t insn, std::ostream *pos)
{
    uint32_t rd   = get_rd(insn);
    uint32_t rs1  = get_rs1(insn);
    uint32_t f3   = get_funct3(insn);


    int32_t  imm  = get_imm_i(insn);
    uint32_t base = static_cast<uint32_t>(regs.get(rs1));
    uint32_t addr = base + imm;


    int32_t loaded = 0;
    std::string mnemonic;


    switch (f3)
    {
    case 0b000: // lb
        mnemonic = "lb";
        loaded   = mem.get8_sx(addr);
        break;


    case 0b001: // lh
        mnemonic = "lh";
        loaded   = mem.get16_sx(addr);
        break;


    case 0b010: // lw
        mnemonic = "lw";
        loaded   = static_cast<int32_t>(mem.get32(addr));
        break;


    case 0b100: // lbu
        mnemonic = "lbu";
        loaded   = static_cast<int32_t>(mem.get8(addr));
        break;


    case 0b101: // lhu
        mnemonic = "lhu";
        loaded   = static_cast<int32_t>(mem.get16(addr));
        break;


    default:
        exec_illegal_insn(insn, pos);
        return;
    }


    if (pos)
    {
        std::string s = render_itype_load(insn, mnemonic);
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;
        *pos << "// " << render_reg(rd)
             << " = mem[" << hex::to_hex0x32(addr) << "] = "
             << hex::to_hex0x32(static_cast<uint32_t>(loaded));
    }


    regs.set(rd, loaded);
    pc += 4;
}


/***************************************************************
Function: rv32i_hart::exec_store


Use:   Stores: sb, sh, sw.
***************************************************************/
void rv32i_hart::exec_store(uint32_t insn, std::ostream *pos)
{
    uint32_t rs1 = get_rs1(insn);
    uint32_t rs2 = get_rs2(insn);
    uint32_t f3  = get_funct3(insn);


    int32_t  imm  = get_imm_s(insn);
    uint32_t base = static_cast<uint32_t>(regs.get(rs1));
    uint32_t addr = base + imm;


    uint32_t rs2_u = static_cast<uint32_t>(regs.get(rs2));
    std::string mnemonic;


    switch (f3)
    {
    case 0b000: // sb
        mnemonic = "sb";
        mem.set8(addr, static_cast<uint8_t>(rs2_u & 0xff));
        break;


    case 0b001: // sh
        mnemonic = "sh";
        mem.set16(addr, static_cast<uint16_t>(rs2_u & 0xffff));
        break;


    case 0b010: // sw
        mnemonic = "sw";
        mem.set32(addr, rs2_u);
        break;


    default:
        exec_illegal_insn(insn, pos);
        return;
    }


    if (pos)
    {
        std::string s = render_stype(insn, mnemonic);
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;
        *pos << "// mem[" << hex::to_hex0x32(addr) << "] = "
             << hex::to_hex0x32(rs2_u);
    }


    pc += 4;
}


/***************************************************************
Function: rv32i_hart::exec_branch


Use:   Branches: beq, bne, blt, bge, bltu, bgeu.
***************************************************************/
void rv32i_hart::exec_branch(uint32_t insn, std::ostream *pos)
{
    uint32_t rs1 = get_rs1(insn);
    uint32_t rs2 = get_rs2(insn);
    uint32_t f3  = get_funct3(insn);


    int32_t  rs1_val = regs.get(rs1);
    int32_t  rs2_val = regs.get(rs2);


    uint32_t pc_before = pc;
    int32_t  imm       = get_imm_b(insn);
    uint32_t target    = pc_before + imm;


    bool take = false;
    std::string mnemonic;


    switch (f3)
    {
    case 0b000: // beq
        mnemonic = "beq";
        take = (rs1_val == rs2_val);
        break;


    case 0b001: // bne
        mnemonic = "bne";
        take = (rs1_val != rs2_val);
        break;


    case 0b100: // blt (signed)
        mnemonic = "blt";
        take = (rs1_val < rs2_val);
        break;


    case 0b101: // bge (signed)
        mnemonic = "bge";
        take = (rs1_val >= rs2_val);
        break;


    case 0b110: // bltu (unsigned)
        mnemonic = "bltu";
        take = (static_cast<uint32_t>(rs1_val)
                < static_cast<uint32_t>(rs2_val));
        break;


    case 0b111: // bgeu (unsigned)
        mnemonic = "bgeu";
        take = (static_cast<uint32_t>(rs1_val)
                >= static_cast<uint32_t>(rs2_val));
        break;


    default:
        exec_illegal_insn(insn, pos);
        return;
    }


    if (pos)
    {
        std::string s = render_btype(pc_before, insn, mnemonic);
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;


        *pos << "// " << render_reg(rs1)
             << " = " << hex::to_hex0x32(static_cast<uint32_t>(rs1_val))
             << ", " << render_reg(rs2)
             << " = " << hex::to_hex0x32(static_cast<uint32_t>(rs2_val))
             << ", ";


        if (take)
        {
            *pos << "br_taken  pc = " << hex::to_hex0x32(target);
        }
        else
        {
            *pos << "br_not_taken  pc = "
                 << hex::to_hex0x32(pc_before + 4);
        }
    }


    if (take)
        pc = target;
    else
        pc = pc_before + 4;
}


/***************************************************************
Function: rv32i_hart::exec_ecall


Use:   Treat ECALL as a halt.
***************************************************************/
void rv32i_hart::exec_ecall(uint32_t insn, std::ostream *pos)
{
    (void)insn;


    if (pos)
    {
        std::string s = "ecall";
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;
        *pos << "// HALT";
    }


    halt = true;
    halt_reason = "ECALL instruction";
}


/***************************************************************
Function: rv32i_hart::exec_csrrx


Use:   CSR ops with register operand: csrrw, csrrs, csrrc.
***************************************************************/
void rv32i_hart::exec_csrrx(uint32_t insn, std::ostream *pos,
                            const std::string &mnemonic)
{
    uint32_t rd   = get_rd(insn);
    uint32_t rs1  = get_rs1(insn);
    uint32_t csr_addr = insn >> 20;


    if (csr_addr >= 4096)
    {
        exec_illegal_insn(insn, pos);
        return;
    }


    uint32_t old_val = csr[csr_addr];
    uint32_t rs1_val = static_cast<uint32_t>(regs.get(rs1));
    uint32_t new_val = old_val;


    if (mnemonic == "csrrw")
    {
        new_val = rs1_val;
    }
    else if (mnemonic == "csrrs")
    {
        if (rs1 != 0)
            new_val = old_val | rs1_val;
    }
    else if (mnemonic == "csrrc")
    {
        if (rs1 != 0)
            new_val = old_val & ~rs1_val;
    }
    else
    {
        exec_illegal_insn(insn, pos);
        return;
    }


    csr[csr_addr] = new_val;


    if (pos)
    {
        std::string s = render_csrrx(insn, mnemonic);
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;


        *pos << "// csr[" << hex::to_hex0x12(csr_addr)
             << "] was " << hex::to_hex0x32(old_val)
             << ", now " << hex::to_hex0x32(new_val);


        if (rd != 0)
        {
            *pos << "; " << render_reg(rd)
                 << " = " << hex::to_hex0x32(old_val);
        }
    }


    if (rd != 0)
        regs.set(rd, static_cast<int32_t>(old_val));


    pc += 4;
}


/***************************************************************
Function: rv32i_hart::exec_csrrxi


Use:   CSR ops with immediate operand:
       csrrwi, csrrsi, csrrci.
***************************************************************/
void rv32i_hart::exec_csrrxi(uint32_t insn, std::ostream *pos,
                             const std::string &mnemonic)
{
    uint32_t rd   = get_rd(insn);
    uint32_t zimm = get_rs1(insn);  // 5-bit
    uint32_t csr_addr = insn >> 20;


    if (csr_addr >= 4096)
    {
        exec_illegal_insn(insn, pos);
        return;
    }


    uint32_t old_val = csr[csr_addr];
    uint32_t new_val = old_val;


    if (mnemonic == "csrrwi")
    {
        new_val = zimm;
    }
    else if (mnemonic == "csrrsi")
    {
        if (zimm != 0)
            new_val = old_val | zimm;
    }
    else if (mnemonic == "csrrci")
    {
        if (zimm != 0)
            new_val = old_val & ~zimm;
    }
    else
    {
        exec_illegal_insn(insn, pos);
        return;
    }


    csr[csr_addr] = new_val;


    if (pos)
    {
        std::string s = render_csrrxi(insn, mnemonic);
        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;


        *pos << "// csr[" << hex::to_hex0x12(csr_addr)
             << "] was " << hex::to_hex0x32(old_val)
             << ", now " << hex::to_hex0x32(new_val);


        if (rd != 0)
        {
            *pos << "; " << render_reg(rd)
                 << " = " << hex::to_hex0x32(old_val);
        }
    }


    if (rd != 0)
        regs.set(rd, static_cast<int32_t>(old_val));


    pc += 4;
}


/***************************************************************
Function: rv32i_hart::exec_ebreak


Use:   Treat EBREAK as a halt.
***************************************************************/
void rv32i_hart::exec_ebreak(uint32_t insn, std::ostream *pos)
{
    (void)insn;


    if (pos)
    {
        std::string s = "ebreak";


        *pos << std::setw(instruction_width)
             << std::setfill(' ') << std::left << s;
        *pos << "// HALT";
    }


    halt = true;
    halt_reason = "EBREAK instruction";
}

