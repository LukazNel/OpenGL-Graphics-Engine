#include <iostream>
#include <bitset>

//Block order: {[Z, Y, X], V}
//             {[(+-Oz)(Oz)(+-Oy)(Oy)(+-Ox)(Ox)][(+-Rz)(Rz)(+-Ry)(Ry)(+-Rx)(Rx)][L][C][+-Z, +-Y, +-X]}

struct block {
  int Coordinates[3];
  unsigned int Colour;
  unsigned int Level;
  int Rotation[3];
  float Offset[3];
};

int main() {
  block Block;
  uint64_t Output[2] = {0, 0};
  int Shift = 0;

  std::cout << "Enter Info: [X Y Z] [C L] [Rx Ry Rz] [Ox Oy Oz]" << std::endl;
  std::cin >> Block.Coordinates[0] >> Block.Coordinates[1] >> Block.Coordinates[2] >> Block.Colour >> Block.Level >> Block.Rotation[0] >> Block.Rotation[1] >> Block.Rotation[2] >> Block.Offset[0] >> Block.Offset[1] >> Block.Offset[2];

 Shift = 1 - 21;
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

  std::cout << std::endl << "Result: " << Output[0] << " " << Output[1] << std::endl;
  return 0;
}
