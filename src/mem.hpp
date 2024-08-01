#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <iostream>
#include <cstdint>

const int MAX_MEM_SIZE=(1<<20)+16;

class Memory{
  private:
  unsigned char mem[MAX_MEM_SIZE];

  public:
  Memory(){
    std::fill(mem, mem+MAX_MEM_SIZE, 0);
  }
  // read from .data and store all the statements
  void init() {
    bool count = false;
    char c;
    unsigned int place = 0, x = 0;
    while ((c = getchar()) != EOF) {
      if (c >= 'a' && c <= 'z')
        c -= 32;
      if (c == '@') {
        std::cin >> std::hex >> place;
      } else if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) {
        if (count) {
          x = x * 16 + (c > '9' ? 10 + c - 'A' : c - '0');
          mem[place++] = x;
          x = 0;
          count ^= 1;
        } else
          x = c > '9' ? 10 + c - 'A' : c - '0', count ^= 1;
      }
    }
  }
  unsigned int fetch(int place) {
    unsigned int ins = 0;
    for (int i = 0; i < 4; ++i)
      ins |= (mem[i + place] << (i * 8));
    return ins;
  }
  unsigned int load(int place, int n) {
    unsigned int x = 0;
    // std::cout << "place= " << place << ' ' << n << '\n';
    for (int i = 0; i < n; ++i)
      x |= mem[i + place] << (i * 8);
    return x;
  }
  void store(int place, unsigned int x, int n) {
    // std::cerr << "store= " << place << ' ' << x << ' ' << n << '\n';
    for (int i = 0; i < n; ++i)
      mem[i + place] = x & 0xff, x >>= 8;
  }
};

#endif // !MEMORY_HPP