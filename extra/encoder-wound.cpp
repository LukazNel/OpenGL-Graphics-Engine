#include <iostream>
#include "block_encode.h"

int main() {
  block Block;
  uint64_t Output[2] = {0, 0};
  int Shift = 0;

  std::cout << "Enter Info: [X Y Z] [C L I] [Rx Ry Rz] [Ox Oy Oz]" << std::endl;
  std::cin >> Block.Coordinates[0] >> Block.Coordinates[1] >> Block.Coordinates[2] >> Block.Colour >> Block.Level >> Block.Intensity >> Block.Rotation[0] >> Block.Rotation[1] >> Block.Rotation[2] >> Block.Offset[0] >> Block.Offset[1] >> Block.Offset[2];
  blockEncode(Block, Output);
  std::cout << "Result: " << Output[0] << " " << Output[1] << std::endl;
  return 0;
}
