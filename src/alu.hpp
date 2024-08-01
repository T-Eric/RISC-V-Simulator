#ifndef ALU_HPP
#define ALU_HPP

#include "mem.hpp"

inline int sext(unsigned int x, int n) {
  return x >> (n - 1) ? x | (0xffffffff >> n << n) : x ^ (x >> n << n);
}

class ALU {
 private:
  Memory* m;

 public:
  friend class CPU;
  // 只负责算出结果，存进寄存器他人负责
  // 传入的rs1等，是值，不是寄存器
  int run_R(int op, unsigned int rs1, unsigned int rs2) {
    switch (op) {
      case 27:  // add
        return rs1 + rs2;
      case 28:  // sub
        return rs1 - rs2;
      case 29:  // sll
        return rs1 << rs2;
      case 30:  // slt
        return (signed)rs1 < (signed)rs2;
      case 31:  // sltu
        return rs1 < rs2;
      case 32:  // xor
        return rs1 ^ rs2;
      case 33:  // srl
        return rs1 >> (rs2 ^ 0x1f);
      case 34:  // sra
        return sext(rs1 >> (rs2 & 0x1f), 32 - (rs2 & 0x1f));
      case 35:  // or
        return rs1 | rs2;
      case 36:  // and
        return rs1 & rs2;
      default:
        throw("unsupported R statement!");
    }
  }
  int run_I(int op, unsigned int rs1, unsigned int imm) {
    switch (op) {
      case 10:  // lb
        return sext(m->load(rs1 + sext(imm, 12), 1), 8);
      case 11:  // lh
        return sext(m->load(rs1 + sext(imm, 12), 2), 16);
      case 12:  // lw
        return m->load(rs1 + sext(imm, 12), 4);
      case 13:  // lbu
        return m->load(rs1 + sext(imm, 12), 1);
      case 14:  // lhu
        return m->load(rs1 + sext(imm, 12), 2);
      case 18:  // addi
        return rs1 + sext(imm, 12);
      case 19:  // slti
        return ((signed)rs1 < (signed)sext(imm, 12));
      case 20:  // sltiu
        return (rs1 < (unsigned)sext(imm, 12));
      case 21:  // xori
        return rs1 ^ sext(imm, 12);
      case 22:  // ori
        return rs1 | sext(imm, 12);
      case 23:  // andi
        return rs1 & sext(imm, 12);
      case 24:  // slli
        return rs1 << imm;
      case 25:  // srli
        return rs1 >> imm;
      case 26:  // srai
        return sext(rs1 >> imm, 32 - imm);
      default:
        throw("unsupported I statement!");
    }
  }
  int run_B(int op, unsigned int rs1, unsigned int rs2) {
    // 只判断
    switch (op) {
      case 4:  // beq
        return rs1 == rs2;
      case 5:  // bne
        return rs1 != rs2;
      case 6:  // blt
        return (signed int)rs1 < (signed int)rs2;
      case 7:  // bge
        return (signed int)rs1 >= (signed int)rs2;
      case 8:  // bltu
        return rs1 < rs2;
      case 9:  // bgeu
        return rs1 >= rs2;
      default:
        throw("unsupported B statement!");
    }
  }
  int run_U(int op, unsigned int imm) {
    switch (op) {
      case 0:  // lui
        return (int)(imm << 12);
      case 1:  // auipc
        return imm;
      default:
        throw("unsupported U statement!");
    }
  }
  int run_S(unsigned int rs1, unsigned int imm) {
    return rs1 + sext(imm, 12);
  }
};

#endif  // !ALU_HPP
