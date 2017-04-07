#include <iostream>
#include <fstream>
#include <bitset>
#include <cstdlib>
#include <cstdint>

//#define FileOut std::cout

struct block {
  int Coordinates[3];
  unsigned int Colour;
  unsigned int Level;
  int Rotation[3];
  float Offset[3];
};

void mortonEncode(block &Block, uint64_t (&Output)[2]);

int main() {
  std::cout << "Enter seed:" << std::endl;
  int Seed;
  std::cin >> Seed;
  srand(Seed);

  std::fstream FileOut("blockarray.h", std::ios::out | std::ios::trunc);

  FileOut << "#ifndef __blockarray_h_" << std::endl
          << "#define __blockarray_h_" << std::endl << std::endl
          << "static const uint64_t BlockArray[10000][2] {" << std::endl;

  for (int i = 0; i < 100; i++) {
    for (int k = 0; k < 100; k++) {
      block Block = {{i, 0, k}, rand() % 1024, rand() % 4, {0, 0, 0}, {0, 0, 0}};
      uint64_t Answer[2] = {0, 0};
      mortonEncode(Block, Answer);
      FileOut << "{" << Answer[0] << ", " << Answer[1] << "}, ";
    }
    FileOut << std::endl;
  }
  FileOut << std::endl << std::endl << "#endif";
  FileOut.close();
  return 0;
}

void mortonEncode(block &Block, uint64_t (&Output)[2]) {
   int Shift = 1 - 21;
  for (int i = 0; i < 3; i++) {
    Shift += 21;
    if (Block.Coordinates[i] < 0) {
      Output[1] |= 0x1 << (i + 1);
      Block.Coordinates[i] *= -1;
    }
    Output[0] |= ((uint64_t)Block.Coordinates[i] & 0x1FFFFF) << Shift;
  }
    
  Output[0] |= 0x1;
  Output[1] |= (Block.Colour & 0x3FF) << 4;
  Output[1] |= (Block.Level & 0x3) << 14;
  
  Shift = 16;
  for (int i = 0; i < 3; i++) {
    int RotateVar = 0;
    if (Block.Rotation[i] < 0) {
      RotateVar |= 0x1 << 7;
      Block.Rotation[i] *= -1;
    }
    RotateVar |= (Block.Rotation[i] & 0x7F);
    Output[1] |= RotateVar << Shift;
    Shift += 8;
  }

  Shift = 40;
  for (int i = 0; i < 3; i++) {
    int OffsetVar = 0;
    if (Block.Offset[i] < 0) {
      OffsetVar |= 0x1 << 7;
      Block.Offset[i] *= -1;
    }
    OffsetVar |= ((int)(Block.Offset[i] * 100) & 0x7F);
    Output[1] |= OffsetVar << Shift;
    Shift += 8;
  }
}
