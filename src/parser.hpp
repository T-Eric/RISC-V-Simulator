#ifndef PARSER_HPP
#define PARSER_HPP

#include "utils.hpp"

// parser拿到一串机器码，要能将自己是何种操作，有哪些变量，给端上来
void R_dec(unsigned int st, unsigned int& opcode, int& rs1, int& rs2, int& imm, int& rd) {
  rd = (st >> 7) & 0x1F;
  rs1 = (st >> 15) & 0x1F;
  rs2 = (st >> 20) & 0x1F;
  imm = 0;
  unsigned int funct3 = (st >> 12) & 0x7;
  switch (funct3) {
    case 0:
      opcode = (st >> 30) ^ 1 ? 27 : 28;
      break;
    case 1:
      opcode = 29;
      break;
    case 2:
      opcode = 30;
      break;
    case 3:
      opcode = 31;
      break;
    case 4:
      opcode = 32;
      break;
    case 5:
      opcode = (st >> 30) ^ 1 ? 33 : 34;
      break;
    case 6:
      opcode = 35;
      break;
    case 7:
      opcode = 36;
      break;
    default:
      throw("unsupported statement!");
      break;
  }
}

void I_dec(unsigned int st, unsigned int& opcode, int& rs1, int& rs2, int& imm, int& rd) {
  rd = (st >> 7) & 0x1F;
  rs1 = (st >> 15) & 0x1F;
  rs2 = -1;
  imm = st >> 20;
  unsigned int funct3 = (st >> 12) & 0x7;
  unsigned int prefix = st & 0x7f;
  switch (funct3) {
    case 0:
      switch (prefix) {
        case 0x67:
          opcode = 3;
          break;
        case 0x3:
          opcode = 10;
          break;
        case 0x13:
          opcode = 18;
          break;
        default:
          throw("unsupported statement!");
          break;
      }
      break;
    case 1:
      opcode = prefix == 0x3 ? 11 : 24;
      break;
    case 2:
      opcode = prefix == 0x3 ? 12 : 19;
      break;
    case 3:
      opcode = 20;
      break;
    case 4:
      opcode = prefix == 0x3 ? 13 : 21;
      break;
    case 5:
      switch (prefix) {
        case 0x3:
          opcode = 14;
          break;
        case 0x13:
          opcode = (st >> 30) ^ 1 ? 25 : 26;
          if (opcode == 26)
            imm &= 0x1f;  // 取出shamt，另两个高位没有数字，而srai前面有0100000
          break;
        default:
          throw("unsupported statement!");
          break;
      }
      break;
    case 6:
      opcode = 22;
      break;
    case 7:
      opcode = 23;
      break;
    default:
      throw("unsupported statement!");
      break;
  }
}

void S_dec(unsigned int st, unsigned int& opcode, int& rs1, int& rs2, int& imm, int& rd) {
  rs1 = (st >> 15) & 0x1F;
  rs2 = (st >> 20) & 0x1F;
  rd = -1;
  imm = ((st >> 7) & 0x1f) | (get_num(st, 31, 25) << 5);
  unsigned int func3 = (st >> 12) & 0x7;
  switch (func3) {
    case 0:
      opcode = 15;
      break;
    case 1:
      opcode = 16;
      break;
    case 2:
      opcode = 17;
      break;
    default:
      throw("unsupported statement!");
      break;
  }
}

void B_dec(unsigned int st, unsigned int& opcode, int& rs1, int& rs2, int& imm, int& rd) {
  rs1 = (st >> 15) & 0x1F;
  rs2 = (st >> 20) & 0x1F;
  rd = 0;
  // imm = (((st >> 8) & 0xf) << 1) | (get_num(st, 30, 25) << 5) | (((st >> 7) & 1) << 11) | (((st >> 31)) & 1 << 12);
  imm=0;
  imm|=((st>>31)&1)<<12;
  imm|=((st>>7)&1)<<11;
  imm|=get_num(st,30,25)<<5;
  imm|=((st>>8)&0xf)<<1;
  unsigned int funct3 = (st >> 12) & 0x7;
  switch (funct3) {
    case 0:
      opcode = 4;
      break;
    case 1:
      opcode = 5;
      break;
    case 4:
      opcode = 6;
      break;
    case 5:
      opcode = 7;
      break;
    case 6:
      opcode = 8;
      break;
    case 7:
      opcode = 9;
      break;
    default:
      break;
  }
}

void U_dec(unsigned int st, unsigned int& opcode, int& rs1, int& rs2, int& imm, int& rd) {
  rd = (st >> 7) & 0x1F;
  rs1 = -1;
  rs2 = -1;
  imm = st >> 12;
  opcode = (st & 0x7f) == 0x37 ? 0 : 1;
}

void J_dec(unsigned int st, unsigned int& opcode, int& rs1, int& rs2, int& imm, int& rd) {
  rd = (st >> 7) & 0x1f;
  rs1 = -1;
  rs2 = -1;
  imm = (((st >> 21) & 0x3ff) << 1) | (((st >> 20) & 1) << 11) | (((st >> 12) & 0xff) << 12) | (((st >> 31) & 1) << 20);
  opcode = 2;
}

struct Statement {
  unsigned int opcode;
  int rs1, rs2, imm, rd;

  Statement(unsigned int st) {
    unsigned int prefix = st & 0x7f;
    if (prefix == 0x37 || prefix == 0x17)
      U_dec(st, opcode, rs1, rs2, imm, rd);
    else if (prefix == 0x6f)
      J_dec(st, opcode, rs1, rs2, imm, rd);
    else if (prefix == 0x63)
      B_dec(st, opcode, rs1, rs2, imm, rd);
    else if (prefix == 0x67 || prefix == 0x3 || prefix == 0x13)
      I_dec(st, opcode, rs1, rs2, imm, rd);
    else if (prefix == 0x23)
      S_dec(st, opcode, rs1, rs2, imm, rd);
    else if (prefix == 0x33)
      R_dec(st, opcode, rs1, rs2, imm, rd);
  }

  int get_rd() { return rd; }
  int get_rs1() { return rs1; }
  int get_rs2() { return rs2; }
  int get_imm() { return imm; }
};

bool is_r(unsigned int opcode) {
  return opcode >= 27 && opcode <= 36;
}
bool is_i(unsigned int opcode) {
  return opcode == 3 || opcode >= 10 && opcode <= 14 || opcode >= 18 && opcode <= 26;
}
bool is_s(unsigned int opcode) {
  return opcode >= 15 && opcode <= 17;
}
bool is_b(unsigned int opcode) {
  return opcode >= 4 && opcode <= 9;
}
bool is_u(unsigned int opcode) {
  return opcode == 0 || opcode == 1;
}
bool is_j(unsigned int opcode) {
  return opcode == 2;
}

#endif  // !PARSER_HPP