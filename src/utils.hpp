#ifndef UTILS_HPP
#define UTILS_HPP

unsigned int get_num(unsigned int s, int hi, int lo) {
  unsigned int mask = ((1 << (hi - lo + 1)) - 1) << lo;
  return (s & mask) >> lo;
}

#endif // !UTILS_HPP