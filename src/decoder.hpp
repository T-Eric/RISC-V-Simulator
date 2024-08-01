#ifndef DEC_HPP
#define DEC_HPP

#include "parser.hpp"
#include "register.hpp"
#include "rob.hpp"

class Decoder {
 private:
  ReorderBuffer* rob;
  ReservationStation* rs;
  LoadStoreBuffer* lsb;
  RegisterFile* rf;

 public:
  friend class ReorderBuffer;
  friend class CPU;
  Decoder() {}
  Decoder(ReorderBuffer* rob, ReservationStation* rs, LoadStoreBuffer* lsb, RegisterFile* rf)
      : rob(rob), rs(rs), lsb(lsb), rf(rf) {}
  inline Statement decode(unsigned int st) {
    return Statement(st);
  }
  bool issue(Statement st, int pc, bool clk) {
    // rob,lsb,rs任何一个满了都不行
    if (rob->full(!clk))
      return false;
    bool is_ls = st.opcode >= 10 && st.opcode <= 17;
    if (is_ls && lsb->full(!clk))
      return false;
    if (!is_ls && rs->full(!clk))
      return false;
    ++rob->size[clk];
    rob->que[rob->tail[clk]][clk] = RoBData(rob->tail[clk], true, st.opcode, 0);
    // if (st.rd != -1)
    //   rob->que[rob->tail[clk]][clk].dest = st.rd;

    RSData* rsd = nullptr;
    if (is_ls) {
      for (int i = 0; i < lsb->MAXSIZE; ++i) {
        if (!lsb->data[i][!clk].busy) {
          rsd = &lsb->data[i][clk];
          rsd->busy = 1;
          lsb->add(clk);
          break;
        }
      }
    } else {
      for (int i = 0; i < rs->MAXSIZE; ++i) {
        if (!rs->data[i][!clk].busy) {
          rsd = &rs->data[i][clk];
          rs->add(clk);
          break;
        }
      }
    }
    rsd->busy = 1;
    rsd->dest = rob->tail[clk];
    rsd->A = st.get_imm();  // 暂存
    rsd->op = st.opcode;

    int rs1 = st.get_rs1();
    int rs2 = st.get_rs2();
    int rd = st.get_rd();
    int op = st.opcode;
    if (is_j(op)) {
      rob->que[rob->tail[clk]][clk].value = pc + 4;
    } else if (op == 3) {  // jalr
      rob->que[rob->tail[clk]][clk].value = pc + 4;
      rob->block[clk] = true;
    } else if (op == 1) {
      rsd->A += pc;
    }
    rsd->qj = rsd->qk = -1;
    if (!is_u(op) && !is_j(op)) {  // 需要rs1的指令
      if (rf->reg.occupied_by[rs1][!clk] != -1) {
        int h = rf->reg.occupied_by[rs1][!clk];
        if (!rob->que[h][!clk].busy) {
          rsd->vj = rob->que[h][!clk].value;
          rsd->qj = -1;
        } else
          rsd->qj = h;
      } else {
        rsd->vj = rf->reg.reg[rs1][!clk];
        rsd->qj = -1;
      }
      if (!rs1) {
        rsd->vj = 0;
        rsd->qj = -1;
      }
    }
    if (is_b(op) || is_s(op) || is_r(op)) {  // 需要rs2的指令
      if (rf->reg.occupied_by[rs2][!clk] != -1) {
        int h = rf->reg.occupied_by[rs2][!clk];
        if (!rob->que[h][!clk].busy) {
          rsd->vk = rob->que[h][!clk].value;
          rsd->qk = -1;
        } else
          rsd->qk = h;
      } else {
        rsd->vk = rf->reg.reg[rs2][!clk];
        rsd->qk = -1;
      }
      if (!rs2) {
        rsd->vk = 0;
        rsd->qk = -1;
      }
    }
    if (is_b(op)) {
      rob->que[rob->tail[clk]][clk].value = (pc & 1) | (rf->pc[!clk] - 4);
      rob->que[rob->tail[clk]][clk].dest = pc & ~1;
    }
    if (!is_b(op) && !is_s(op)) {  // 需要rd
      rf->reg.occupied_by[rd][clk] = rob->tail[clk];
      rob->que[rob->tail[clk]][clk].dest = rd;
    }
    rob->tail[clk] = (rob->tail[clk] + 1) % rob->MAXSIZE;
    return true;
  }
};

#endif