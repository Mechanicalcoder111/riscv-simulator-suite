/********************************************************************************************
RISC-V Simulator

Programmer:  Aasim Ghani

Purpose:
    Implements the 'rv32i_decode' class, which provides a pure software decoder
    for the RV32I instruction set. It:
      - Extracts instruction fields (opcode, rd, rs1, rs2, funct3, funct7, and
        the various immediate formats).
      - Decodes 32-bit instruction words into human-readable assembly mnemonics.
      - Formats operands (registers, immediates, and PC-relative targets) using
        helper rendering functions and the hex formatting tools inherited from
        the 'hex' class.

    No printing is done directly in this file; instead, decode() returns a
    std::string that the caller (main/disassemble) prints.
********************************************************************************************/


#include "rv32i_decode.h"
#include <sstream>
#include <cassert>


using std::string;


/***************************************************************
Function: rv32i_decode::decode


Use:      Disassembles a single 32-bit RV32I instruction word into
          a human-readable assembly string.


Arguments:
    addr - Address (PC) from which the instruction was fetched.
    insn - 32-bit instruction word to decode.


Returns:
    A std::string containing either the decoded instruction
    (mnemonic and operands) or an error message for unknown/
    unimplemented instructions.
***************************************************************/
std::string rv32i_decode::decode(uint32_t addr, uint32_t insn)
{
    switch (get_opcode(insn))
    {
    case rv32i_decode::opcode_lui:
        return render_lui(insn);


    case rv32i_decode::opcode_auipc:
        return render_auipc(insn);


    case rv32i_decode::opcode_jal:
        return render_jal(addr, insn);


    case rv32i_decode::opcode_jalr:
        return render_jalr(insn);


    case rv32i_decode::opcode_btype:
        switch(get_funct3(insn))
        {
            case 0b000: return render_btype(addr, insn, "beq");
            case 0b001: return render_btype(addr, insn, "bne");
            case 0b100: return render_btype(addr, insn, "blt");
            case 0b101: return render_btype(addr, insn, "bge");
            case 0b110: return render_btype(addr, insn, "bltu");
            case 0b111: return render_btype(addr, insn, "bgeu");
            default:    return rv32i_decode::render_illegal_insn();
        }


    case rv32i_decode::opcode_load:
        switch(get_funct3(insn))
        {
            case 0b000: return render_itype_load(insn, "lb");
            case 0b001: return render_itype_load(insn, "lh");
            case 0b010: return render_itype_load(insn, "lw");
            case 0b100: return render_itype_load(insn, "lbu");
            case 0b101: return render_itype_load(insn, "lhu");
            default:    return rv32i_decode::render_illegal_insn();


        }
   
    case rv32i_decode::opcode_store:
        switch(get_funct3(insn))
        {
            case 0b000: return render_stype(insn, "sb");
            case 0b001: return render_stype(insn, "sh");
            case 0b010: return render_stype(insn, "sw");
           
            default:    return rv32i_decode::render_illegal_insn();
        }
   
    case rv32i_decode::opcode_alu_imm:
        switch(get_funct3(insn))
        {
            case 0b000:
                return render_itype_alu(insn, "addi", get_imm_i(insn));
           
            case 0b010:
                return render_itype_alu(insn, "slti", get_imm_i(insn));
           
            case 0b011:
                return render_itype_alu(insn, "sltiu", get_imm_i(insn));


            case 0b100:
                return render_itype_alu(insn, "xori", get_imm_i(insn));
           
            case 0b110:
                return render_itype_alu(insn, "ori", get_imm_i(insn));


            case 0b111:
                return render_itype_alu(insn, "andi", get_imm_i(insn));
           
            case 0b001:
                if (get_funct7(insn) == 0b0000000)
                    return render_itype_alu(insn, "slli", get_imm_i(insn) & 0x1f);
                return render_illegal_insn();


            case 0b101:
                if (get_funct7(insn) == 0b0000000)
                    return render_itype_alu(insn, "srli", get_imm_i(insn) & 0x1f);
               
                if (get_funct7(insn) == 0b0100000)
                    return render_itype_alu(insn, "srai", get_imm_i(insn) & 0x1f);
                return render_illegal_insn();
            default:    
                return render_illegal_insn();
        }


    case rv32i_decode::opcode_alu_reg:
        switch(get_funct3(insn))
        {
            case 0b000:
                if (get_funct7(insn) == 0b0000000)
                    return render_rtype(insn, "add");
                if (get_funct7(insn) == 0b0100000)
                    return render_rtype(insn, "sub");  
                return render_illegal_insn();
               
            case 0b001:
                return render_rtype(insn, "sll");  
 
            case 0b010:
                return render_rtype(insn, "slt");
           
            case 0b011:
                return render_rtype(insn, "sltu");
           
            case 0b100:
                return render_rtype(insn, "xor");


            case 0b101:
                if (get_funct7(insn) == 0b0000000)
                    return render_rtype(insn, "srl");
                if (get_funct7(insn) == 0b0100000)
                    return render_rtype(insn, "sra");  
                return render_illegal_insn();


            case 0b110:
                return render_rtype(insn, "or");
           
            case 0b111:
                return render_rtype(insn, "and");


            default:
                return render_illegal_insn();
        }


    case rv32i_decode::opcode_system:
        switch(get_funct3(insn))
        {
            case 0b000:
                if (insn == 0x00000073)
                    return render_mnemonic("ecall");
                if (insn == 0x00100073)
                    return render_mnemonic("ebreak");
                return render_illegal_insn();


            case 0b001:
                return render_csrrx(insn, "csrrw");
           
            case 0b010:
                return render_csrrx(insn, "csrrs");
           
            case 0b011:
                return render_csrrx(insn, "csrrc");
           
            case 0b101: // csrrwi
                return render_csrrxi(insn, "csrrwi");
           
            case 0b110:
                return render_csrrxi(insn, "csrrsi");


            case 0b111:
                return render_csrrxi(insn, "csrrci");
               
            default:
                return render_illegal_insn();
        }


    default:
        return render_illegal_insn();
    }


    assert(0 && "unrecognized opcode");
    return render_illegal_insn();
}


/***************************************************************
Function: rv32i_decode::get_opcode


Use:      Extracts the opcode field (bits [6:0]) from a 32-bit
          instruction.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Unsigned 32-bit value containing the opcode (0..0x7f).
***************************************************************/
uint32_t rv32i_decode::get_opcode(uint32_t insn)
{
    return insn & 0x7f;
}


/***************************************************************
Function: rv32i_decode::get_rd


Use:      Extracts the rd field (destination register) from the
          instruction.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Unsigned 32-bit integer 0..31 representing rd.
***************************************************************/
uint32_t rv32i_decode::get_rd(uint32_t insn)
{
    return (insn >> 7) & 0x1f;
}


/***************************************************************
Function: rv32i_decode::get_rs1


Use:      Extracts the rs1 field (source register 1) from the
          instruction.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Unsigned 32-bit integer 0..31 representing rs1.
***************************************************************/
uint32_t rv32i_decode::get_rs1(uint32_t insn)
{
    return (insn >> 15) & 0x1f;
}


/***************************************************************
Function: rv32i_decode::get_rs2


Use:      Extracts the rs2 field (source register 2) from the
          instruction.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Unsigned 32-bit integer 0..31 representing rs2.
***************************************************************/
uint32_t rv32i_decode::get_rs2(uint32_t insn)
{
    return (insn >> 20) & 0x1f;
}


/***************************************************************
Function: rv32i_decode::get_funct3


Use:      Extracts the funct3 field from the instruction.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Unsigned 32-bit integer 0..7 representing funct3.
***************************************************************/
uint32_t rv32i_decode::get_funct3(uint32_t insn)
{
    return (insn >> 12) & 0x7;
}


/***************************************************************
Function: rv32i_decode::get_funct7


Use:      Extracts the funct7 field from the instruction.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Unsigned 32-bit integer 0..0x7f representing funct7.
***************************************************************/
uint32_t rv32i_decode::get_funct7(uint32_t insn)
{
    return (insn >> 25) & 0x7f;
}


/***************************************************************
Function: rv32i_decode::get_imm_i


Use:      Extracts the I-type immediate (imm_i) from the given
          instruction and sign-extends it to 32 bits.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Signed 32-bit imm_i value.
***************************************************************/
int32_t rv32i_decode::get_imm_i(uint32_t insn)
{
    int32_t imm = (insn >> 20) & 0xfff;


    // sign ext 12-bit imm
    if (imm & 0x800)
        imm |= 0xfffff000;


    return imm;
}


/***************************************************************
Function: rv32i_decode::get_imm_u


Use:      Extracts the U-type immediate (imm_u) from the given
          instruction. The low 12 bits are zero in the encoded
          form.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Signed 32-bit imm_u value with bits [31:12] from the
    instruction and bits [11:0] cleared.
***************************************************************/
int32_t rv32i_decode::get_imm_u(uint32_t insn)
{
    return static_cast<int32_t>(insn & 0xfffff000);
}


/***************************************************************
Function: rv32i_decode::get_imm_b


Use:      Extracts the B-type branch offset (imm_b) from the
          encoded instruction fields and sign-extends it.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Signed 32-bit branch offset in bytes.
***************************************************************/
int32_t rv32i_decode::get_imm_b(uint32_t insn)
{
    int32_t bit12    = (insn >> 31) & 0x1;
    int32_t bits10_5 = (insn >> 25) & 0x3f;
    int32_t bits4_1  = (insn >> 8)  & 0x0f;
    int32_t bit11    = (insn >> 7)  & 0x1;


    int32_t imm = (bit12    << 12)
                | (bit11    << 11)
                | (bits10_5 << 5)
                | (bits4_1  << 1);


    if (bit12)
        imm |= 0xffffe000;


    return imm;
}


/***************************************************************
Function: rv32i_decode::get_imm_s


Use:      Extracts the S-type store offset (imm_s) from the
          instruction fields and sign-extends it.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Signed 32-bit store offset in bytes.
***************************************************************/
int32_t rv32i_decode::get_imm_s(uint32_t insn)
{
    int32_t imm_s = (insn & 0xfe000000) >> (25-5);
    imm_s |= (insn & 0x00000f80) >> (7-0);


    if (insn & 0x80000000)
        imm_s |= 0xfffff000;
    return imm_s;
}


/***************************************************************
Function: rv32i_decode::get_imm_j


Use:      Extracts the J-type jump offset (imm_j) from the
          instruction fields and sign-extends it.


Arguments:
    insn - 32-bit instruction word.


Returns:
    Signed 32-bit jump offset in bytes.
***************************************************************/
int32_t rv32i_decode::get_imm_j(uint32_t insn)
{
    int32_t bit20 = (insn >> 31) & 0x1;
    int32_t bits10_1 = (insn >> 21) & 0x3ff;
    int32_t bit11 = (insn >> 20) & 0x1;
    int32_t bits19_12 = (insn >> 12) & 0xff;


    int32_t imm = (bit20 << 20)
                | (bits19_12 << 12)
                | (bit11 << 11)
                | (bits10_1 << 1);


    if (bit20)
        imm |= 0xffe00000;


    return imm;
}


/***************************************************************
Function: rv32i_decode::render_illegal_insn


Use:      Returns a standard error string for an unimplemented
          or illegal instruction.


Arguments:
    None.


Returns:
    The string "ERROR: UNIMPLEMENTED INSTRUCTION".
***************************************************************/
string rv32i_decode::render_illegal_insn()
{
    return "ERROR: UNIMPLEMENTED INSTRUCTION";
}


/***************************************************************
Function: rv32i_decode::render_mnemonic


Use:      Renders the instruction mnemonic, padding it with
          spaces to mnemonic_width characters (except for
          ecall/ebreak, which are returned without padding).


Arguments:
    mnemonic - Instruction mnemonic to render.


Returns:
    A std::string containing the mnemonic, padded on the right
    when appropriate.
***************************************************************/
string rv32i_decode::render_mnemonic(const std::string &mnemonic)
{
    if (mnemonic == "ecall" || mnemonic == "ebreak")
        return mnemonic;


    std::ostringstream os;
    os << mnemonic;
    while ((int)os.str().size() < mnemonic_width)
        os << ' ';
    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_reg


Use:      Renders a register number as "xN".


Arguments:
    r - Register number (0..31).


Returns:
    A std::string in the form "x5", "x0", etc.
***************************************************************/
string rv32i_decode::render_reg(int r)
{
    std::ostringstream os;
    os << 'x' << r;
    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_base_disp


Use:      Renders a base+displacement addressing operand in the
          form "imm(rs1)".


Arguments:
    rs1 - Register number used as base.
    imm - Signed displacement.


Returns:
    A std::string such as "12(x1)" or "-8(x5)".
***************************************************************/
string rv32i_decode::render_base_disp(uint32_t rs1, int32_t imm)
{
    std::ostringstream os;
    os << imm << "(" << render_reg(rs1) << ")";
    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_lui


Use:      Renders the LUI instruction.


Arguments:
    insn - 32-bit LUI instruction word.


Returns:
    A std::string of the form "lui     xN,0xabcde".
***************************************************************/
std::string rv32i_decode::render_lui(uint32_t insn)
{
    std::ostringstream os;

    uint32_t rd    = get_rd(insn);
    int32_t  imm_u = get_imm_u(insn);   // upper immediate already shifted left 12

    // For disassembly we want:  "lui x4,0xabcde"
    // where 0xabcde = imm_u >> 12 (20-bit value).
    uint32_t imm20 = (uint32_t(imm_u) >> 12) & 0xFFFFF;

    os << render_mnemonic("lui")
       << render_reg(rd)
       << ",0x"
       << std::hex << std::nouppercase
       << imm20;

    return os.str();
}

/***************************************************************
Function: rv32i_decode::render_auipc


Use:      Renders the AUIPC instruction.


Arguments:
    insn - 32-bit AUIPC instruction word.


Returns:
    A std::string of the form "auipc   xN,0xabcde".
***************************************************************/
std::string rv32i_decode::render_auipc(uint32_t insn)
{
    std::ostringstream os;

    uint32_t rd    = get_rd(insn);
    int32_t  imm_u = get_imm_u(insn);   // upper immediate already shifted left 12
    uint32_t imm20 = (uint32_t(imm_u) >> 12) & 0xFFFFF;

    // Disassembly format:  "auipc x4,0xabcde"
    os << render_mnemonic("auipc")
       << render_reg(rd)
       << ",0x"
       << std::hex << std::nouppercase
       << imm20;

    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_jal


Use:      Renders the JAL instruction, including the absolute
          PC-relative target address.


Arguments:
    addr - Address of the JAL instruction.
    insn - 32-bit JAL instruction word.


Returns:
    A std::string of the form "jal     xN,0xXXXXXXXX".
***************************************************************/
std::string rv32i_decode::render_jal(uint32_t addr, uint32_t insn)
{
    uint32_t rd = get_rd(insn);
    int32_t imm = get_imm_j(insn);


    std::ostringstream os;
    os << rv32i_decode::render_mnemonic("jal")
       << rv32i_decode::render_reg(rd) << ","
       << rv32i_decode::to_hex0x32(addr + imm);


    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_jalr


Use:      Renders the JALR instruction with base+displacement
          addressing.


Arguments:
    insn - 32-bit JALR instruction word.


Returns:
    A std::string of the form "jalr    xN,imm(xM)".
***************************************************************/
std::string rv32i_decode::render_jalr(uint32_t insn)
{
    uint32_t rd    = get_rd(insn);
    uint32_t rs1  = get_rs1(insn);
    int32_t imm_i    = get_imm_i(insn);


    std::ostringstream os;


    os << rv32i_decode::render_mnemonic("jalr")
        << rv32i_decode::render_reg(rd) << ","
        << rv32i_decode::render_base_disp(rs1, imm_i);


    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_btype


Use:      Renders any B-type branch instruction (beq, bne, etc.)
          using a shared format.


Arguments:
    addr      - Address of the branch instruction.
    insn      - 32-bit branch instruction word.
    mnemonic  - String name of the branch instruction.


Returns:
    A std::string such as "beq     x1,x2,0xXXXXXXXX".
***************************************************************/
std::string rv32i_decode::render_btype(uint32_t addr, uint32_t insn, const std::string &mnemonic)
{
    uint32_t rs1 = get_rs1(insn);
    uint32_t rs2 = get_rs2(insn);
    int32_t imm  = get_imm_b(insn);


    std::ostringstream os;


    os << rv32i_decode::render_mnemonic(mnemonic)
       << rv32i_decode::render_reg(rs1) << ","
       << rv32i_decode::render_reg(rs2) << ","
       << rv32i_decode::to_hex0x32(addr + imm);


    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_itype_load


Use:      Renders any I-type load instruction (lb, lh, lw, lbu,
          lhu) using a shared format.


Arguments:
    insn      - 32-bit load instruction word.
    mnemonic  - Load mnemonic string ("lb", "lh", etc.).


Returns:
    A std::string such as "lw      x4,1234(x0)".
***************************************************************/
std::string rv32i_decode::render_itype_load(uint32_t insn, const std::string &mnemonic)
{
    uint32_t rd  = get_rd(insn);
    uint32_t rs1 = get_rs1(insn);
    int32_t imm  = get_imm_i(insn);


    std::ostringstream os;


    os << rv32i_decode::render_mnemonic(mnemonic)
        << rv32i_decode::render_reg(rd) << ","
        << rv32i_decode::render_base_disp(rs1, imm);


    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_stype


Use:      Renders any S-type store instruction (sb, sh, sw)
          using a shared format.


Arguments:
    insn      - 32-bit store instruction word.
    mnemonic  - Store mnemonic string ("sb", "sh", "sw").


Returns:
    A std::string such as "sw      x4,1234(x0)".
***************************************************************/
std::string rv32i_decode::render_stype(uint32_t insn, const std::string &mnemonic)
{
    uint32_t rs1 = get_rs1(insn);
    uint32_t rs2 = get_rs2(insn);
    int32_t imm  = get_imm_s(insn);


    std::ostringstream os;


    os << rv32i_decode::render_mnemonic(mnemonic)
        << rv32i_decode::render_reg(rs2) << ","
        << rv32i_decode::render_base_disp(rs1, imm);


    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_itype_alu


Use:      Renders I-type ALU instructions such as addi, slti,
          sltiu, xori, ori, andi, and shift-immediate forms.


Arguments:
    insn      - 32-bit I-type ALU instruction word.
    mnemonic  - Instruction mnemonic.
    imm       - Immediate value (or shift amount).


Returns:
    A std::string such as "addi    x1,x2,10".
***************************************************************/
std::string rv32i_decode::render_itype_alu(uint32_t insn, const std::string &mnemonic, int32_t imm)
{
    uint32_t rd    = get_rd(insn);
    uint32_t rs1 = get_rs1(insn);


    std::ostringstream os;


    os << rv32i_decode::render_mnemonic(mnemonic)
       << rv32i_decode::render_reg(rd) << ","
       << rv32i_decode::render_reg(rs1) << ","
       << imm;


    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_rtype


Use:      Renders R-type ALU instructions (add, sub, sll, slt,
          sltu, xor, srl, sra, or, and) using a shared format.


Arguments:
    insn      - 32-bit R-type instruction word.
    mnemonic  - Instruction mnemonic.


Returns:
    A std::string such as "add     x3,x1,x2".
***************************************************************/
std::string rv32i_decode::render_rtype(uint32_t insn, const std::string &mnemonic)
{
    uint32_t rd  = get_rd(insn);
    uint32_t rs1 = get_rs1(insn);    
    uint32_t rs2 = get_rs2(insn);


    std::ostringstream os;
   
    os << rv32i_decode::render_mnemonic(mnemonic)
       << rv32i_decode::render_reg(rd) << ","
       << rv32i_decode::render_reg(rs1) << ","
       << rv32i_decode::render_reg(rs2);


   
    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_csrrx


Use:      Renders CSR register instructions that use a register
          operand (csrrw, csrrs, csrrc).


Arguments:
    insn      - 32-bit CSR instruction word.
    mnemonic  - Instruction mnemonic.


Returns:
    A std::string such as "csrrw   x20,0x000,x22".
***************************************************************/
std::string rv32i_decode::render_csrrx(uint32_t insn, const std::string &mnemonic)
{
    uint32_t rd  = get_rd(insn);
    uint32_t rs1 = get_rs1(insn);    
   
    uint32_t csr = insn >> 20;


    std::ostringstream os;
    os << rv32i_decode::render_mnemonic(mnemonic)
       << rv32i_decode::render_reg(rd) << ","
       << rv32i_decode::to_hex0x12(csr) << ","
       << rv32i_decode::render_reg(rs1);


    return os.str();
}


/***************************************************************
Function: rv32i_decode::render_csrrxi


Use:      Renders CSR instructions that use an immediate value
          (zimm) instead of a register operand (csrrwi, csrrsi,
          csrrci).


Arguments:
    insn      - 32-bit CSR instruction word.
    mnemonic  - Instruction mnemonic.


Returns:
    A std::string such as "csrrwi  x29,0x0fff,10".
***************************************************************/
std::string rv32i_decode::render_csrrxi(uint32_t insn, const std::string &mnemonic)
{
    uint32_t rd  = get_rd(insn);
    uint32_t csr = insn >> 20;
    uint32_t zimm = get_rs1(insn);


    std::ostringstream os;
    os << rv32i_decode::render_mnemonic(mnemonic)
       << rv32i_decode::render_reg(rd) << ","
       << rv32i_decode::to_hex0x12(csr) << ","
       << zimm;


    return os.str();
}

