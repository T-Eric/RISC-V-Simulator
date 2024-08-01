#ifndef PRED_HPP
#define PRED_HPP

#include "rob.hpp"

// 哈希以重命名，去掉最后两位，往上取6位
class Predictor {
 private:
  const static int maxSize = 1 << 6;
  int sum=0, success=0;
  int stat[maxSize];

 public:
  friend class ReorderBuffer;
  friend class CPU;
  Predictor() {
    for (int i = 0; i < maxSize; ++i)
        stat[i] = 0b10;
  }
  inline int hash(int pc) {
    return (pc >> 2) & 0x3f;
  }
  bool get_prediction(int pc) {
    int i = hash(pc);
    return (stat[i] >> 1) & 1;
  }// 10,11=jump
  void feedback(int pc, bool taken) {
    int i = hash(pc);
    if (taken)
      ++success;
    bool jump = (get_prediction(pc) == taken);
    if (jump && stat[i] < 0b11)
      ++stat[i];
    if (!jump && stat[i])
      --stat[i];
  }
  bool predict(int pc) {
    ++sum;
    int i = hash(pc);
    return (stat[i] >> 1) & 1;
  }
};

#endif  // !PRED_HPP
