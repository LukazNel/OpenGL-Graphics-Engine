#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdint>

#include "../game/src/external/include/stb_image.h"

struct image {
  int Width;
  int Height;
  std::vector<std::vector<int>> ColourID;
};

struct layer {
  std::string Name;
  bool IsVisible;
  bool IsLocked;
  int Size[3];
  int Corner[3];
};

struct voxelobject {
  int Dimensions[3];
  int LayerCount;
  int ImageCount;
  std::vector<image> Images;
  std::vector<layer> Layers;  
};

struct block {
  int Coordinates[3];
  unsigned int Colour;
  unsigned int Level;
  int Rotation[3];
  float Offset[3];
};

void encode(block Block, uint64_t (&Output)[2]);
int findColour(std::vector<unsigned char> InputColour);

int main(int argc, char* argv[])  {
  std::string FileName;
  if (argc != 2) {
    std::cout << "Usage: convert-pnx [.pnx file]\n";
    std::exit(EXIT_FAILURE);
  } else FileName = argv[1];


  std::fstream FileOut("blockarray", std::ios::out | std::ios::trunc | std::ios::binary);

  std::ifstream File(FileName, std::ios::binary);
  voxelobject VoxelObject;
  for (int i = 0; i < 3; i++)
    File.read((char*)&VoxelObject.Dimensions[i], sizeof(int));
  File.read((char*)&VoxelObject.LayerCount, sizeof(int));
  File.read((char*)&VoxelObject.ImageCount, sizeof(int));

  for (int i = 0; i < VoxelObject.ImageCount; i++) {
    int Length = 0;
    File.read((char*)&Length, sizeof(int));
    std::vector<char> Image(Length);
    File.read(Image.data(), sizeof(unsigned char) * Length);
    std::string ImageName = "pnx_images/image" + std::to_string(i) + ".png";
    std::ofstream Out(ImageName);
    Out.write(Image.data(), Image.size());
    Out.close();
    image PostImage;
    unsigned char* Raw = stbi_load(ImageName.c_str(), &PostImage.Width, &PostImage.Height, nullptr, 4);
    if (PostImage.Width == 0 || PostImage.Height == 0) {
      std::cout << "Error occurred\n";
      std::exit(EXIT_FAILURE);
    }
    std::vector<unsigned char> Vector;
    Vector.assign(Raw, Raw + PostImage.Width * PostImage.Height * 4);
    PostImage.ColourID.resize(PostImage.Height, std::vector<int>(PostImage.Width));
    std::vector<std::vector<std::vector<unsigned char>>> ColourArray(PostImage.Height, std::vector<std::vector<unsigned char>>(PostImage.Width, std::vector<unsigned char>(4)));
    int Count = 0;
    for (int j = 0; j < PostImage.Height; j++)
      for (int k = 0; k < PostImage.Width; k++) {
        for (int l = 0; l < 4; l ++)
          ColourArray[j][k][l] = Vector[Count++];
        if (ColourArray[j][k][3] == 255)
          PostImage.ColourID[j][k] = findColour(ColourArray[j][k]);
        else PostImage.ColourID[j][k] = -1;
      }
    stbi_image_free(Raw);
    VoxelObject.Images.push_back(PostImage);
  }
  
  for (int i = 0; i < VoxelObject.LayerCount; i++) {
    layer Layer;

    int NameLength = 0;
    File.read((char*)&NameLength, sizeof(int));
    char* String = new char[NameLength];
    File.read(String, NameLength);
    Layer.Name = String;
    delete [] String;

    File.read((char*)&Layer.IsVisible, sizeof(char));
    File.read((char*)&Layer.IsLocked, sizeof(char));

    for (int i = 0; i < 3; i++)
      File.read((char*)&Layer.Size[i], sizeof(int));

    int LayerCorner[3] = {0, 0, 0};
    for (int i = 0; i < 3; i++)
      File.read((char*)&Layer.Corner[i], sizeof(int));

    for (int x = Layer.Size[0] + Layer.Corner[0] - 1; x > Layer.Corner[0] - 1; x--) {
      int ImageID = 0;
      File.read((char*)&ImageID, sizeof(int));
      for (int z = 0; z < Layer.Size[2]; z++) {
        for (int y = 0; y < Layer.Size[1]; y++) {
          if (VoxelObject.Images[ImageID].ColourID.at(z).at(y) != -1) {
            block Block {{x, 1 - y + Layer.Corner[1], 1 - z + Layer.Corner[2]}, VoxelObject.Images[ImageID].ColourID[z][y], 1, {0, 0, 0}, {0, 0, 0}};
            uint64_t Answer[2] = {0, 0};
            encode(Block, Answer);
            FileOut.write((char*)Answer, 16);
          }
        }
      }
    }
    VoxelObject.Layers.push_back(Layer);
  }
  File.close();
  FileOut.close();
  return 0;
}

int findColour(std::vector<unsigned char> InputColour) {
  int Width, Height;
  unsigned char* Raw = stbi_load("colours.png", &Width, &Height, nullptr, 4);
  std::vector<unsigned char> Vector;
  Vector.assign(Raw, Raw + Width * Height * 4);
  stbi_image_free(Raw);
  std::vector<std::vector<unsigned char>> ColourArray(Width * Height, std::vector<unsigned char>(4));
  int Count = 0;
  for (int i = 0; i < Width * Height; i++)
    for (int j = 0; j < 4; j++)
      ColourArray[i][j] = Vector[Count++];
  int Index = std::distance(ColourArray.begin(), std::find_if(ColourArray.begin(), ColourArray.end(), 
        [&](const std::vector<unsigned char>& Colour) {
          return Colour == InputColour;
        }));
  return Index;
}

void encode(block Block, uint64_t (&Output)[2]) {
  int Shift = 0;

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
}
