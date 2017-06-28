#include <iostream>
#include <cstdint>

uint64_t splitBy3(unsigned int a) {
    uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
    uint64_t Constant = (0x1F << 48) | 0xFFFF;
    //Constant = 0x1F0000000FFFF;
    x = (x | x << 32) & Constant;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
    Constant = (0x1F << 48) | 0xFF0000FF;
    //Constant = 0x1f0000ff0000ff;
    x = (x | x << 16) & Constant;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
    Constant = (0x100F00F << 36) | 0xF00F00F;
    //Constant = 0x100f00f00f00f00f;
    x = (x | x << 8) & Constant; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    Constant = (0x10C30C3 << 36) | 0xC30C30C3;
    //Constant = 0x10c30c30c30c30c3;
    x = (x | x << 4) & Constant; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    Constant = (0x924924 << 33) | 0x49249249;
    //Constant = 0x1249249249249249;
    x = (x | x << 2) & Constant;
    return x;
}
 
uint64_t mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z) {
    uint64_t answer = 0;
    answer |= splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
    return answer;
}

int main() {
  int X, Y, Z;
  std::cout << "Enter x y z: " << std::endl;
  std::cin >> X >> Y >> Z;
  uint64_t Answer = mortonEncode_magicbits(X, Y, Z);
  std::cout << std::endl << Answer << std::endl;
  return 0;
}
