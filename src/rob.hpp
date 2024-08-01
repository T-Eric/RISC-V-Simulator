#ifndef ROB_HPP
#define ROB_HPP

#include "alu.hpp"
#include "mem.hpp"
#include "parser.hpp"
#include "predictor.hpp"
#include "register.hpp"
#include "rs.hpp"

struct RoBData {
  unsigned int id;
  bool busy;
  unsigned int op;
  unsigned int dest;
  unsigned int value;
  RoBData() {}
  RoBData(unsigned int id, bool busy, unsigned int op, unsigned int value)
      : id(id), busy(busy), op(op), value(value), dest(0) {}
  friend class Decoder;
};

class ReorderBuffer {
 private:
  int size[2], head[2], tail[2];
  bool block[2];
  // size：当前大小，head：队列头，tail：队列尾，block：阻塞赋值标识
  // 之所以阻塞赋值
  const static int MAXSIZE = 64;
  RoBData que[MAXSIZE][2];
  ReservationStation* rs;
  LoadStoreBuffer* lsb;
  RegisterFile* rf;
  Memory* mem;
  ALU* alu;
  Predictor* pre;

 public:
  friend class ReservationStation;
  friend class LoadStoreBuffer;
  friend class Decoder;
  friend class CPU;
  ReorderBuffer() {}

  explicit ReorderBuffer(ReservationStation* rs, LoadStoreBuffer* lsb, RegisterFile* rf, Memory* mem, ALU* alu, Predictor* pre)
      : rs(rs), lsb(lsb), rf(rf), mem(mem), alu(alu), pre(pre) {}

  void update(bool clk) {
    size[!clk] = size[clk];
    head[!clk] = head[clk];
    tail[!clk] = tail[clk];
    block[!clk] = block[clk];
    for (int i = 0; i < MAXSIZE; i++)
      que[i][!clk] = que[i][clk];
  }

  bool full(bool clk) { return size[clk] == MAXSIZE; }

  void rs_execute(bool clk) {
    RoBData* rbd = nullptr;
    RSData* rsd = nullptr;
    for (int i = 0; i < MAXSIZE; ++i) {
      if (!rs->data[i][!clk].busy || rs->data[i][!clk].qj != -1 || rs->data[i][!clk].qk != -1)
        continue;
      rsd = &rs->data[i][clk];
      rbd = &que[rsd->dest][clk];
      rbd->busy = false;

      if (rbd->dest == -1) {
        int a = 1;
      }

      if (is_r(rsd->op)) {
        if (rbd->dest)
          rbd->value = alu->run_R(rsd->op, rsd->vj, rsd->vk);
      } else if (is_u(rsd->op)) {
        if (rbd->dest)  ////////
          rbd->value = alu->run_U(rsd->op, rsd->A);
      } else if (is_i(rsd->op)) {
        if (rsd->op == 3) {
          // 直接跳转即可
          rf->pc[clk] = (rsd->vj + sext(rsd->A, 12)) & ~1;
        } else {
          if (rbd->dest)
            rbd->value = alu->run_I(rsd->op, rsd->vj, rsd->A);
        }
      } else if (is_b(rsd->op)) {
        rbd->value ^= alu->run_B(rsd->op, rsd->vj, rsd->vk);
      }
      rs->data[i][clk].busy = false;
      --rs->size[clk];
      rs->release(rsd->dest, rbd->value, clk);
      lsb->release(rsd->dest, rbd->value, clk);
    }
  }

  void lsb_execute(bool clk) {
    RoBData* rbd = nullptr;
    RSData* lsd = nullptr;
    for (int i = 0; i < MAXSIZE; ++i) {
      if (!lsb->data[i][!clk].busy || lsb->data[i][!clk].qj != -1 || lsb->data[i][!clk].qk != -1)
        continue;
      lsd = &lsb->data[i][clk];
      rbd = &que[lsd->dest][clk];
      bool flag = false;
      unsigned int h = lsd->dest;
      if (is_i(rbd->op)) {
        for (int i = 0, k = head[!clk]; i < size[!clk] && k != h; ++i, k = (k + 1) % MAXSIZE) {
          if (k ^ h && is_s(que[k][!clk].op)) {
            flag = true;
            break;
          }
        }
      } else {
        for (int i = 0, k = head[!clk]; i < size[!clk] && k != h; ++i, k = (k + 1) % MAXSIZE) {
          if (k ^ h && (is_s(que[k][!clk].op) || (que[k][clk].op >= 10 && que[k][clk].op <= 14))) {
            flag = true;
            break;
          }
        }
      }
      if (flag)
        continue;
      if (lsb->data[i][clk].busy < 3) {
        ++lsb->data[i][clk].busy;
        continue;
      }
      rbd->busy = 0;
      if (is_s(rbd->op)) {
        rbd->dest = lsd->vj + sext(lsd->A, 12);
        rbd->value = lsd->vk;
      } else {
        rbd->value = alu->run_I(lsd->op, lsd->vj, lsd->A);
      }
      lsb->data[i][clk].busy = 0;
      --lsb->size[clk];
      lsb->release(lsd->dest, rbd->value, clk);
      rs->release(lsd->dest, rbd->value, clk);
    }
  }

  // 提交，将结果输入
  // 对于跳转语句，假如要跳转，将rob,rs,lsb,reg的值全都清掉，
  // 且直到执行完毕前暂停指令读取
  bool commit(bool clk) {
    if (!size[!clk] || que[head[!clk]][!clk].busy)
      return true;  // 无需提交
    RoBData* rbd = &que[head[clk]][clk];
    head[clk] = (head[clk] + 1) % MAXSIZE;
    --size[clk];
    rbd->busy = false;

    if (rbd->op == 3)
      // jalu，之前有block过，需要解
      block[clk] = false;
    if (is_b(rbd->op)) {
      if (rbd->value & 1) {
        // 需要跳转，全部清除
        pre->feedback(rbd->value & (~1), false);  ////////这个位置pc到底要传入什么？
        rf->pc[clk] = rbd->dest;
        clear(clk);
        rs->clear(clk);
        lsb->clear(clk);
        rf->clear(clk);
        return false;  // 告诉cpu统一操作
      } else {
        pre->feedback(rbd->value&(~1), true);
      }
    } else if (is_s(rbd->op)) {
      int bits = rbd->op == 15 ? 1 : (rbd->op == 16 ? 2 : 4);
      mem->store(rbd->dest, rbd->value, bits);
    } else {
      // 普通计算，解除寄存器写占用
      if (rbd->dest)
        rf->reg.reg[rbd->dest][clk] = rbd->value;
      if (rf->reg.occupied_by[rbd->dest][clk] == rbd->id)
        rf->reg.occupied_by[rbd->dest][clk] = -1;
      rs->release(rbd->id, rbd->value, clk);
      lsb->release(rbd->id, rbd->value, clk);
    }
    return true;
  }
  void clear(bool clk) {
    size[clk] = size[!clk] = head[clk] = head[!clk] = block[clk] = block[!clk] = tail[clk] = tail[!clk] = 0;
    for (int i = 0; i < MAXSIZE; ++i)
      que[i][clk].busy = que[i][!clk].busy = false;
  }
};

#endif  // !ROB_HPP
