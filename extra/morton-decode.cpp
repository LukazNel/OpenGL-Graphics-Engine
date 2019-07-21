#include <iostream>
#include <cstdint>

unsigned int regroup (uint64_t x) {
  x = Constant & (x >> 2 | x);
  Constant = 0x49249249 | (0x924924 >> 13);
  x = Constant & (x >> 4 | x);
  Constant = 
}
