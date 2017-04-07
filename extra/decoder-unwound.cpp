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
  uint64_t Coordinates = 0;
  uint64_t Info = 0;
  std::cout << "Enter number: ";
  std::cin >> Coordinates >> Info;

  if (Coordinates & 0x1) {
    Coordinates >>= 1;
    Info >>= 1;
    
    Block.Coordinates[0] = Coordinates & 0x1FFFFF;
    if (Info & 0x1)
      Block.Coordinates[0] *= -1;
    Coordinates >>= 21;
    Info >>= 1;

    Block.Coordinates[1] = Coordinates & 0x1FFFFF;
    if (Info & 0x1)
      Block.Coordinates[1] *= -1;
    Coordinates >>= 21;
    Info >>= 1;

    Block.Coordinates[2] = Coordinates & 0x1FFFFF;
    if (Info & 0x1)
      Block.Coordinates[2] *= -1;
    //Coordinates >>= 21;
    Info >>= 1;

    //uint64_t MortonIndex = mortonEncode_magicbits(Block.Coordinates.x, Block.Coordinates.y, Block.Coordinates.z);

    Block.Colour = Info & 0x3FF;
    Info >>= 10;
    Block.Level = Info & 0x3;
    Info >>= 2;

    Block.Rotation[0] = Info & 0x7F;
    Info >>= 7;
    if (Info & 0x1)
      Block.Rotation[0] *= -1;
    Info >>= 1;

    Block.Rotation[1] = Info & 0x7F;
    Info >>= 7;
    if (Info & 0x1)
      Block.Rotation[1] *= -1;
    Info >>= 1;

    Block.Rotation[2] = Info & 0x7F;
    Info >>= 7;
    if (Info & 0x1)
      Block.Rotation[2] *= -1;
    Info >>= 1;

    Block.Offset[0] = (float)(Info & 0x7F) / 100;
    Info >>= 7;
    if (Info & 0x1)
      Block.Offset[0] *= -1;
    Info >>= 1;

    Block.Offset[1] = (float)(Info & 0x7F) / 100;
    Info >>= 7;
    if (Info & 0x1)
      Block.Offset[1] *= -1;
    Info >>= 1;

    Block.Offset[2] = (float)(Info & 0x7F) / 100;
    Info >>= 7;
    if (Info & 0x1)
      Block.Offset[2] *= -1;
  }
  std::cout << "Result: [X Y Z] [C L] [Rx Ry Rz] [Ox Oy Oz]" << std::endl;
  std::cout << Block.Coordinates[0] << " " << Block.Coordinates[1] << " " << Block.Coordinates[2] << " " << Block.Colour << " " << Block.Level << " " << Block.Rotation[0] << " " << Block.Rotation[1] << " " << Block.Rotation[2] << " " << Block.Offset[0] << " " << Block.Offset[1] << " " << Block.Offset[2] << std::endl;
  return 0;
}
