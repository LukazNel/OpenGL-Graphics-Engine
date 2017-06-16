#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>

struct block {
  int Coordinates[3];
  unsigned int Colour;
  unsigned int Level;
  int Rotation[3];
  float Offset[3];
};

void mortonEncode(block &Block, uint64_t (&Output)[2]);

int main(int argc, char* argv[]) {
  int XSize, YSize, ZSize, Seed;
  if (argc != 5) {
    std::cout << "usage: blocker [x] [y] [z] [seed]" << std::endl;
    std::exit(EXIT_FAILURE);
  } else {
    XSize = std::stoi(argv[1]);
    YSize = std::stoi(argv[2]);
    ZSize = std::stoi(argv[3]);
    Seed = std::stoi(argv[4]);
  }
  srand(Seed);

  std::fstream FileOut("blockarray", std::ios::out | std::ios::trunc | std::ios::binary);

  for (int x = 0; x < XSize; x++) {
    for (int y = 0; y < YSize; y++) {
      for (int z = 0; z < ZSize; z++) {
        block Block = {{x, y, z}, x, 1, {0, 0, 0}, {0, 0, 0}};
        uint64_t Answer[2] = {0, 0};
        mortonEncode(Block, Answer);
        FileOut.write((char*)Answer, 16);
      }
    }
  }
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
    uint64_t RotateVar = 0;
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
    uint64_t OffsetVar = 0;
    if (Block.Offset[i] < 0) {
      OffsetVar |= 0x1 << 7;
      Block.Offset[i] *= -1;
    }
    OffsetVar |= ((int)(Block.Offset[i] * 100) & 0x7F);
    Output[1] |= OffsetVar << Shift;
    Shift += 8;
  }
}
