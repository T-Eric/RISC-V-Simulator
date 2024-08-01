#ifndef RS_HPP
#define RS_HPP

#include <iostream>
#include <vector>
#include "parser.hpp"
#include "register.hpp"
#include "rob.hpp"

struct RSData {
  unsigned int busy;    // 兼具lsb中计算步数的作用
  unsigned int op;      // opcode
  unsigned int qj, qk;  // 操作数依赖的指令在保留站的id
  unsigned int vj, vk;  // 操作数的值
  unsigned int A;       // 这条指令执行完后算出来的值
  int dest;             // rob序号
};

struct RSBase {
 public:
  const static int MAXSIZE = 64;
  RSData data[MAXSIZE][2];
  int size[2];  // 当前已有大小
};

class ReservationStation : public RSBase {
 private:
 public:
  friend class LoadStoreBuffer;
  friend class ReorderBuffer;

  void update(bool clk) {
    size[!clk] = size[clk];
    for (int i = 0; i < MAXSIZE; ++i) {
      data[i][!clk] = data[i][clk];
    }
  }
  // 返回值是在保留站中的位置
  void add(bool clk) {
    // // 直接插入到最小的空位，需要保证自己没满
    // int p = 0;
    // for (; p < MAXSIZE; ++p)
    //   if (!data[p][clk].busy)
    //     break;
    // RSData* rsd = &data[p][clk];
    // rsd->busy = true;
    // rsd->op = st.opcode;
    // // v：直接拉出寄存器中的值，即使不需要也先这么做
    // // 实际上保存这个时刻的寄存器值，就是实现了寄存器重命名
    // int rs1 = st.get_rs1();
    // int rs2 = st.get_rs2();
    // int rd = st.get_rd();
    // // 假如不需要rs2，rs2=imm
    // rsd->vj = rs1 == -1 ? rf->get(rs1, clk) : 0;
    // rsd->vk = rs2 == -1 ? rf->get(rs2, clk) : st.get_imm();
    // // 如果dest值为-1，说明其要修改pc
    // rsd->dest = rd == -1 ? rf->get(rd, clk) : -1;

    // // q: 遍历两个保留站的rd，看看两个rs是否有被占用
    // // 假如在LSB中被占，将这个数加上100比较稳妥，因为0和-0都是0
    // int qj=-1,qk=-1,qjl=-1,qkl=-1;

    size[clk]++;
    // return p;
  }

  bool full(bool clk) {
    return size[clk] == MAXSIZE;
  }
  // 充当bus，消除依赖关系
  // 参考上个时刻的值
  void release(unsigned int id, int value, int clk) {
    for (int i = 0; i < MAXSIZE; ++i) {
      if (!data[i][!clk].busy)
        continue;
      if (data[i][!clk].qj == id) {
        data[i][clk].qj = -1;
        data[i][clk].vj = value;
      }
      if (data[i][!clk].qk == id) {
        data[i][clk].qk = -1;
        data[i][clk].vk = value;
      }
    }
  }
  void clear(bool clk) {
    size[clk] = size[!clk] = 0;
    for (int i = 0; i < MAXSIZE; ++i)
      data[i][clk].busy = data[i][!clk].busy = false;
  }
};

// LSB专门用来处理load和store类指令
// 当然，执行交给CPU
// 貌似需要有自己独立的计算模块？
class LoadStoreBuffer : public RSBase {
 private:
 public:
  friend class ReservationStation;
  friend class ReorderBuffer;

  explicit LoadStoreBuffer() {}

  void update(bool clk) {
    size[!clk] = size[clk];
    for (int i = 0; i < MAXSIZE; ++i) {
      data[i][!clk] = data[i][clk];
    }
  }

  void add(bool clk) {
    // 直接插入到最小的空位，需要保证自己没满
    // int p = 0;
    // for (; p < MAXSIZE; ++p)
    //   if (!data[p][clk].busy)
    //     break;
    // RSData* rsd = &data[p][clk];
    // rsd->busy = true;
    // rsd->op = st.opcode;
    // // v：直接拉出寄存器中的值，即使不需要也先这么做
    // // 实际上保存这个时刻的寄存器值，就是实现了寄存器重命名
    // int rs1 = st.get_rs1();
    // int rs2 = st.get_rs2();
    // int rd = st.get_rd();
    // // 假如不需要rs2，rs2=imm
    // rsd->vj = rs1 == -1 ? rf->get(rs1, clk) : 0;
    // rsd->vk = rs2 == -1 ? rf->get(rs2, clk) : st.get_imm();
    // // 如果dest值为-1，说明其要修改pc
    // rsd->dest = rd == -1 ? rf->get(rd, clk) : -1;

    // // q: 遍历两个保留站的rd，看看两个rs是否有被占用
    // // 假如在LSB中被占，将这个数加上100比较稳妥，因为0和-0都是0
    // int qj = -1, qk = -1, qjl = -1, qkl = -1;

    size[clk]++;
    // return p;
  }

  bool full(bool clk) {
    return size[clk] == MAXSIZE;
  }

  void release(unsigned int id, int value, int clk) {
    for (int i = 0; i < MAXSIZE; ++i) {
      if (!data[i][!clk].busy)
        continue;
      if (data[i][!clk].qj == id) {
        data[i][clk].qj = -1;
        data[i][clk].vj = value;
      }
      if (data[i][!clk].qk == id) {
        data[i][clk].qk = -1;
        data[i][clk].vk = value;
      }
    }
  }

  void clear(bool clk) {
    size[clk] = size[!clk] = 0;
    for (int i = 0; i < MAXSIZE; ++i)
      data[i][clk].busy = data[i][!clk].busy = false;
  }
};
#endif  // !RS_HPP
