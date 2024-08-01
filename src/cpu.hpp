#ifndef CPU_HPP
#define CPU_HPP

#include <functional>
#include <random>
#include "decoder.hpp"
#include "rob.hpp"

// 两个计算模块，alu和Address unit

class CPU {
 private:
  ALU alu;
  Decoder dec;
  RegisterFile rf;
  Memory mem;
  ReservationStation rs;
  LoadStoreBuffer lsb;
  ReorderBuffer rob;
  Predictor pre;

  bool clk = 0;
  bool changeflag[2];
  bool fetchflag[2];
  bool pcflag[2];
  bool break_out;
  unsigned int ins[2], change[2], changepc[2];

  std::random_device rd;
  std::function<void()> f[5];

 public:
  explicit CPU() {
    alu.m = &mem;
    rob = ReorderBuffer(&rs, &lsb, &rf, &mem, &alu, &pre);
    dec = Decoder(&rob, &rs, &lsb, &rf);
    f[0] = [this]() -> void { fetch(clk); };
    f[1] = [this]() -> void { break_out = decode(clk); };
    f[2] = [this]() -> void { rob.rs_execute(clk); };
    f[3] = [this]() -> void { rob.lsb_execute(clk); };
    f[4] = [this]() -> void { if(!rob.commit(clk)) clear(clk); };
  }

  void update(bool clk) {
    ins[!clk] = ins[clk];
    change[!clk] = change[clk];
    changeflag[!clk] = changeflag[clk];
    changepc[!clk] = changepc[clk];
    pcflag[!clk] = pcflag[clk];
    fetchflag[!clk] = fetchflag[clk];
    rf.update(clk);
    rob.update(clk);
    rs.update(clk);
    lsb.update(clk);
  }
  void clear(bool clk) {
    fetchflag[clk] = 0;
    fetchflag[!clk] = 1;
    ins[clk] = ins[!clk] = change[clk] = change[!clk] = changepc[clk] = changepc[!clk] = changeflag[clk] = changeflag[!clk] = pcflag[clk] = pcflag[!clk] = break_out = 0;
  }
  void fetch(bool clk) {
    if (rob.block[!clk] || break_out)
      return;
    if (fetchflag[!clk]) {
      ins[clk] = 0;
      return;
    }
    if (pcflag[!clk]) {
      rf.pc[!clk] = changepc[!clk];
      pcflag[clk] = false;
    }
    ins[clk] = mem.fetch(rf.pc[!clk]);
    rf.pc[clk] = rf.pc[!clk] + 4;
    fetchflag[clk] = false;
  }
  bool issue(Statement st, bool clk, bool res) {
    unsigned int op = st.opcode;
    if (!is_b(op))
      return dec.issue(st, rf.pc[!clk] - 4, clk);
    else
      return dec.issue(st, (rf.pc[!clk] - 4 + (res ? 4 : sext(st.get_imm(), 13))) | res, clk);
  }
  bool decode(bool clk) {
    if (rob.block[!clk] || break_out)
      return false;
    if (changeflag[!clk])
      ins[!clk] = change[!clk];
    changeflag[clk] = fetchflag[clk] = pcflag[clk] = false;
    if (ins[!clk] == 0x0ff00513)  // li a0,255
      return true;
    if (!ins[!clk])
      return false;

    Statement st(dec.decode(ins[!clk]));
    unsigned int op = st.opcode;

    bool res = false;
    if (is_b(op))
      res = pre.predict(rf.pc[!clk] - 4);
    if (!issue(st, clk, res)) {
      rf.pc[clk] = rf.pc[!clk];
      pcflag[clk] = changeflag[clk] = fetchflag[!clk] = fetchflag[clk] = true;
      change[clk] = ins[!clk];
      return false;
    }
    if (is_j(op)) {
      changepc[clk] = rf.pc[!clk] - 4 + sext(st.get_imm(), 21);
      change[clk] = 0;
      pcflag[clk] = changeflag[clk] = true;
    } else if (is_b(op)) {
      changepc[clk] = rf.pc[!clk] - 4 + (res ? sext(st.get_imm(), 13) : 4);
      change[clk] = 0;
      pcflag[clk] = changeflag[clk] = true;
    } else if (op == 3) {
      change[clk] = 0;
      changeflag[clk] = fetchflag[clk] = true;
    }
    return false;
  }
  void execute() {
    mem.init();
    rf.clear(0), rf.clear(1);
    break_out = false;
    int step = 0;
    while (true) {
      // std::cout << "step: " << std::dec << ++step << " pc: " << std::hex << rf.pc[clk] << " ins: " << ins[clk] << '\n';
      // for (int i = 0; i < 32; ++i) {
      //   std::cout << rf.reg.reg[i][clk] << " | ";
      // }
      // std::cout << "\n\n";

      // if (rf.pc[clk] == 0x1054) {
      //   int a = 1;
      // }

      // if (ins[clk] == 0x5ca63) {
      //   int a = 1;
      // }

      // if (step == 22) {
      //   int a = 1;
      // }

      // break_out = decode(clk);
      // fetch(clk);
      // rob.rs_execute(clk);
      // rob.lsb_execute(clk);
      // if (!rob.commit(clk))
      //   clear(clk);
      std::shuffle(f, f + 5, rd);
      for (int i = 0; i < 5; ++i)
        f[i]();
      if (break_out && !rob.size[clk]) {
        // std::cout << "Finish!" << std::endl;
        // std::cout << "clock: " << std::dec << step << std::endl;
        // std::cout << "success/predicts: " << pre.success << " / " << pre.sum << std::endl;
        // std::cout << "answer: " << std::dec << (rf.reg.reg[10][0]&0xff);
        std::cout<<std::dec<<(rf.reg.reg[10][0]&0xff);
        break;
      }
      update(clk);
      clk ^= 1;
    }
  }
  // 3个步骤：fetch&issue, decode&execute, commit
};

#endif  // !CPU_HPP