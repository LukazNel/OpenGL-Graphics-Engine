#version 450 core 

#extension GL_NV_gpu_shader5 : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
//8

struct block {
  ivec3 Coordinates;
  vec3 Colour;
  uint Level;
  ivec3 Rotation;
  ivec3 Offset;
};

layout(std140) uniform InputBuffer {
  u64vec2 InputArray[262144]; // (8x8x8)(8x8x8)
};

layout(std430) buffer StorageBuffer {
  block BlockArray[1000000]; // (60MB)
};

layout(std140) uniform ColourUniform {
  vec3 ColourArray[1022]; // Change to vec3 later!
};

// method to seperate bits from a given integer 3 positions apart
uint64_t splitBy3(unsigned int a) {
    uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
    uint64_t ConstantA = (0x1F << 48) | 0xFFFF;
    x = (x | x << 32) & ConstantA;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
    uint64_t ConstantB = (0x1F << 48) | 0xFF0000FF;
    x = (x | x << 16) & ConstantB;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
    uint64_t ConstantC = (0x100F00F << 36) | 0xF00F00F;
    x = (x | x << 8) & ConstantC; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    uint64_t ConstantD = (0x10C30C3 << 36) | 0xC30C30C3;
    x = (x | x << 4) & ConstantD; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    uint64_t ConstantE = (0x924924 << 33) | 0x49249249;
    x = (x | x << 2) & ConstantE;
    return x;
}
 
uint64_t mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z) {
    uint64_t answer = 0;
    answer |= splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
    return answer;
}

void main(void) {
  block DecodedBlock;
  uint64_t Coordinates = InputArray[gl_GlobalInvocationID].y;
  uint64_t Info = InputArray[gl_GlobalInvocationID].x;

  if ((bool)(Coordinates & 0x1)) {
    Coordinates >>= 1;
    Info >>= 1;
    DecodedBlock.Coordinates.x = (int)(Coordinates & 0x1FFFFF);
    Info >>= 1;
    if ((bool)(Info & 0x1))
      DecodedBlock.Coordinates.x *= -1;
    Coordinates >>= 21;

    DecodedBlock.Coordinates.y = (int)(Coordinates & 0x1FFFFF);
    Info >>= 1;
    if ((bool)(Info & 0x1))
      DecodedBlock.Coordinates.y *= -1;
    Coordinates >>= 21;

    DecodedBlock.Coordinates.z = (int)(Coordinates & 0x1FFFFF);
    Info >>= 1;
    if ((bool)(Info & 0x1))
      DecodedBlock.Coordinates.z *= -1;

    uint64_t MortonIndex = mortonEncode_magicbits((uint)(DecodedBlock.Coordinates.x * 2), (uint)(DecodedBlock.Coordinates.y * 2), (uint)(DecodedBlock.Coordinates.z * 2)); // Times two because negative is removed.

    DecodedBlock.Colour = ColourArray[(uint)(Info & 0x3FF)];
    Info >>= 10;
    DecodedBlock.Level = (uint)(Info & 0x3);
    Info >>= 2;

    DecodedBlock.Rotation.x = (int)(Info & 0x7F);
    Info >>= 7;
    if ((bool)(Info & 0x1))
      DecodedBlock.Rotation.x *= -1;
    Info >>= 1;

    DecodedBlock.Rotation.y = (int)(Info & 0x7F);
    Info >>= 7;
    if ((bool)(Info & 0x1))
      DecodedBlock.Rotation.y *= -1;
    Info >>= 1;

    DecodedBlock.Rotation.z = (int)(Info & 0x7F);
    Info >>= 7;
    if ((bool)(Info & 0x1))
      DecodedBlock.Rotation.z *= -1;
    Info >>= 1;

    DecodedBlock.Offset.x = (int)(Info & 0x7F);
    Info >>= 7;
    if ((bool)(Info & 0x1))
      DecodedBlock.Offset.x *= -1;
    Info >>= 1;

    DecodedBlock.Offset.y = (int)(Info & 0x7F);
    Info >>= 7;
    if ((bool)(Info & 0x1))
      DecodedBlock.Offset.y *= -1;
    Info >>= 1;

    DecodedBlock.Offset.z = (int)(Info & 0x7F);
    Info >>= 7;
    if ((bool)(Info & 0x1))
      DecodedBlock.Offset.z *= -1;

    BlockInfo[gl_GlobalInvocationID] = DecodedBlock;
  }
}
