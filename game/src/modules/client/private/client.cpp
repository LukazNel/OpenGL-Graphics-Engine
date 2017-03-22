#include "client.h"

client::client() {
}

void client::start() {
  setName("Client", this);
  addFunction("setCameraPointers", &client::setCameraPointers);
  addFunction("update", &client::update);
  addFunction("setState", &client::setState);
}

void client::setCameraPointers(float* CSMatrix, float* WSMatrix, float* Position, std::atomic<bool>* DataIsReady) {
  RendererData.CSMatrix = CSMatrix;
  RendererData.WSMatrix = WSMatrix;
  RendererData.Position = Position;
  RendererData.DataIsReady = DataIsReady;
}

void client::update() {
  //glm::vec3 CameraPosition(1, 1, 1);
  glm::mat4 PerspectiveMatrix = glm::perspective(glm::radians(45.0f), (float)1 /*(float)(WindowData.WindowWidth / WindowData.WindowHeight)*/, 0.1f, 100.0f);
  glm::mat4 CameraMatrix = glm::lookAt(CameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  glm::mat4 WSMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.03125)); // 0.03125
  glm::mat4 CSMatrix = PerspectiveMatrix * CameraMatrix;

  if (RendererData.DataIsReady->load() == false) {
    std::copy(glm::value_ptr(CSMatrix), glm::value_ptr(CSMatrix) + 16, RendererData.CSMatrix);
    std::copy(glm::value_ptr(WSMatrix), glm::value_ptr(WSMatrix) + 16, RendererData.WSMatrix);
    std::copy(glm::value_ptr(CameraPosition), glm::value_ptr(CameraPosition) + 3, RendererData.Position);
    RendererData.DataIsReady->store(true);
  }
}

void client::setState(std::string Type, bool State) {
  if (Type == "Forward" && State)
    CameraPosition.z -= 1;
  if (Type == "Backward" && State)
    CameraPosition.z += 1;
  if (Type == "Left" && State)
    CameraPosition.x += 1;
  if (Type == "Right" && State)
    CameraPosition.x -= 1;
}

void client::shutDown() {
}

client::~client() {
}
