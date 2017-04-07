#include <iostream>

struct block {
  int Coordinates[3];
  unsigned int Colour;
  unsigned int Level;
  int Rotation[3];
  float Offset[3];
};

int main() {
  block Block;
  uint64_t Input[2] = {0, 0};
  int Shift = 0;

  std::cout << "Enter number: ";
  std::cin >> Input[0] >> Input[1];

  Shift = 1;
  for (int i = 0; i < 3; i++) {
    Block.Coordinates[i] = (Input[0] >> Shift) & 0x1FFFFF;
    if ((Input[1] >> (i + 1)) & 0x1) {
      Block.Coordinates[i] *= -1;
    }
    Shift += 21;
  }

  Block.Colour = (Input[1] >> 4) & 0x3FF;
  Block.Level = (Input[1] >> 14) & 0x3;
  
  Shift = 16;
  for (int i = 0; i < 3; i++) {
    int RotateVar = 0;
    RotateVar = (Input[1] >> Shift) & 0xFF;
    Block.Rotation[i] = RotateVar & 0x7F;
    if ((RotateVar >> 7) & 0x1) {
      Block.Rotation[i] *= -1;
    }
    Shift += 8;
  }

  Shift = 40;
  for (int i = 0; i < 3; i++) {
    int OffsetVar = 0;
    OffsetVar = (Input[1] >> Shift) & 0xFF;
    Block.Offset[i] = (float)(OffsetVar & 0x7F) / 100;
    if ((OffsetVar >> 7) & 0x1) {
      Block.Offset[i] *= -1;
    }
    Shift += 8;
  }

  std::cout << "Result: [X Y Z] [C L] [Rx Ry Rz] [Ox Oy Oz]" << std::endl;
  std::cout << Block.Coordinates[0] << " " << Block.Coordinates[1] << " " << Block.Coordinates[2] << " " << Block.Colour << " " << Block.Level << " " << Block.Rotation[0] << " " << Block.Rotation[1] << " " << Block.Rotation[2] << " " << Block.Offset[0] << " " << Block.Offset[1] << " " << Block.Offset[2] << std::endl;
  return 0;
}
