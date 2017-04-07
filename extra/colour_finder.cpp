#include <iostream>
#include <vector>
#include "game/src/external/include/stb_image.h"
#include <algorithm>
#include <iterator>

int main() {
  std::cout << "Enter Colour [R][G][B]:\n";
  int x, y, z;
  std::vector<unsigned char> InputColour(3);
  std::cin >> x >> y >> z;
  InputColour = {x, y, z, 255};
  int Width, Height;
  unsigned char* Raw = stbi_load("colours.png", &Width, &Height, nullptr, 4);
  std::vector<unsigned char> Vector;
  Vector.assign(Raw, Raw + Width * Height * 4);
  std::vector<std::vector<unsigned char>> ColourArray(Width * Height, std::vector<unsigned char>(4));
  int Count = 0;
  for (int i = 0; i < Width * Height; i++)
    for (int j = 0; j < 4; j++)
      ColourArray[i][j] = Vector[Count++];
  int Index = std::distance(ColourArray.begin(), std::find_if(ColourArray.begin(), ColourArray.end(), 
        [&](const std::vector<unsigned char>& Colour) {
          return Colour == InputColour;
        }));
  if (Index == ColourArray.size())
    std::cout << "Colour does not exist in array.\n";
  else
    std::cout << "Colour found at index " << Index << std::endl;
  stbi_image_free(Raw);
  return 0;
}
