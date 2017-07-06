#ifndef _BLOCK_ENCODE_H_
#define _BLOCK_ENCODE_H_

//Block order: {[Z, Y, X], V}
//             {[(+-Oz)(Oz)(+-Oy)(Oy)(+-Ox)(Ox)][(+-Rz)(Rz)(+-Ry)(Ry)(+-Rx)(Rx)][I][L][C]}

#include <cstdint>

struct block {
  int Coordinates[3];
  unsigned int Colour;
  unsigned int Level;
  float Intensity;
  int Rotation[3];
  float Offset[3];
};

void blockEncode(block Block, uint64_t (&Output)[2])  { 
  int Shift = 1;
  for (int i = 0; i < 3; i++) {
    uint64_t CoordinatesVar = 0;
    if (Block.Coordinates[i] < 0) {
      CoordinatesVar |= 0x1 << 20;
      Block.Coordinates[i] *= -1;
    }
    CoordinatesVar |= (Block.Coordinates[i] & 0xFFFFF);
    Output[0] |= (CoordinatesVar << Shift);
    Shift += 21;
  }

  Output[0] |= 0x1;
  Output[1] |= (Block.Colour & 0x3FF);
  Output[1] |= (Block.Level & 0x3) << 10;
  if (Block.Intensity <= 1.28)
    Block.Intensity *= 100;
  Output[1] |= ((unsigned int)Block.Intensity & 0x7F) << 12;
  Shift = 19;

  for (int i = 0; i < 3; i++) {
    uint64_t RotateVar = 0;
    if (Block.Rotation[i] < 0) {
      RotateVar |= 0x1 << 6;
      Block.Rotation[i] *= -1;
    }
    if (Block.Rotation[i] >= 360) 
      Block.Rotation[i] -= 360;
    else if (Block.Rotation[i] >= 270)
      Block.Rotation[i] -= 270;
    else if (Block.Rotation[i] >= 180)
      Block.Rotation[i] -= 180;
    else if (Block.Rotation[i] >= 90)
      Block.Rotation[i] -= 90;
    RotateVar |= (Block.Rotation[i] & 0x3F);
    Output[1] |= (RotateVar << Shift);
    Shift += 7;
  }

  for (int i = 0; i < 3; i++) {
    uint64_t OffsetVar = 0;
    if (Block.Offset[i] < 0) {
      OffsetVar |= 0x1 << 7;
      Block.Offset[i] *= -1;
    }
    if (Block.Offset[i] <= 1.28)
      Block.Offset[i] *= 100;
    OffsetVar |= ((unsigned int)(Block.Offset[i]) & 0x7F);
    Output[1] |= (OffsetVar << Shift);
    Shift += 8;
  }
}

#endif // _BLOCK_ENCODE_H_
