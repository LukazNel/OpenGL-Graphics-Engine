#include "client.h"

client::client() {
  CameraData.WSMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.03125));
  CameraData.Position = {0, 0, 2};
  CameraData.Front = {0, 0, -1};
  CameraData.Up = {0, 1, 0};
  ClientState.LastMouseX = 320;
  ClientState.LastMouseY = 240;
}

void client::start() {
  setName("Client", this);
  addFunction("setCameraPointers", &client::setCameraPointers);
  addFunction("update", &client::update);
  addFunction("setState", &client::setState);
  addFunction("setMouse", &client::setMouse);
}

void client::setCameraPointers(float* CSMatrix, float* WSMatrix, float* SkyboxMatrix, float* Position, std::atomic<bool>* DataIsReady) {
  RendererData.CSMatrix = CSMatrix;
  RendererData.WSMatrix = WSMatrix;
  RendererData.SkyboxMatrix = SkyboxMatrix;
  RendererData.Position = Position;
  RendererData.DataIsReady = DataIsReady;
}

void client::update() {
  bool DataIsReady = false;
  int WindowWidth = 640;
  int WindowHeight = 480;
  float NewDeltaTime = 0;
  immediateRequest("Window", "getDataImmediate", &DataIsReady, &WindowWidth, &WindowHeight, &NewDeltaTime);
  if (WindowData.WindowWidth != WindowWidth || WindowData.WindowHeight != WindowHeight) {
    CameraData.PerspectiveMatrix = glm::perspective(glm::radians(50.0f), (float)(WindowWidth / WindowHeight), 0.1f, 100.0f);
    WindowData.WindowWidth = WindowWidth;
    WindowData.WindowHeight = WindowHeight;
  }
  WindowData.DeltaTime = (WindowData.DeltaTime + NewDeltaTime) / 2;
  updatePosition();
  CameraData.CameraMatrix = glm::lookAt(CameraData.Position, CameraData.Position + CameraData.Front, CameraData.Up);
  CameraData.SkyboxMatrix = CameraData.PerspectiveMatrix * glm::lookAt(glm::vec3(CameraData.Position.x, CameraData.Position.y + 0.25, CameraData.Position.z), CameraData.Position + glm::vec3(CameraData.Front.x, -CameraData.Front.y, -CameraData.Front.z), CameraData.Up);
  
  //if (anything changes)
  //  WSMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.03125));
  
  CameraData.CSMatrix = CameraData.PerspectiveMatrix * CameraData.CameraMatrix;

  if (RendererData.DataIsReady->load() == false) {
    std::copy(glm::value_ptr(CameraData.CSMatrix), glm::value_ptr(CameraData.CSMatrix) + 16, RendererData.CSMatrix);
    std::copy(glm::value_ptr(CameraData.WSMatrix), glm::value_ptr(CameraData.WSMatrix) + 16, RendererData.WSMatrix);
    std::copy(glm::value_ptr(CameraData.SkyboxMatrix), glm::value_ptr(CameraData.SkyboxMatrix) + 16, RendererData.SkyboxMatrix);
    std::copy(glm::value_ptr(CameraData.Position), glm::value_ptr(CameraData.Position) + 3, RendererData.Position);
    RendererData.DataIsReady->store(true);
  }
}

void client::setState(std::string Type, bool State) {
  if (Type == "Forward")
    ClientState.Keyboard[0] = State;
  else if (Type == "Backward")
    ClientState.Keyboard[1] = State;
  else if (Type == "Left")
    ClientState.Keyboard[2] = State;
  else if (Type == "Right")
    ClientState.Keyboard[3] = State;
  else if (Type == "Up")
    ClientState.Keyboard[4] = State;
  else if (Type == "Down")
    ClientState.Keyboard[5] = State;
}


void client::setMouse(int MouseX, int MouseY) {
  int MouseOffsetX = MouseX - ClientState.LastMouseX;
  int MouseOffsetY = ClientState.LastMouseY - MouseY;
 
  ClientState.LastMouseX = MouseX;
  ClientState.LastMouseY = MouseY;

  float Sensitivity = 0.005 * WindowData.DeltaTime;
  MouseOffsetX *= Sensitivity;
  MouseOffsetY *= Sensitivity;

  ClientState.Yaw += MouseOffsetX;
  ClientState.Pitch += MouseOffsetY;

  if (ClientState.Pitch > 89)
    ClientState.Pitch = 89;
  else if (ClientState.Pitch < -89)
    ClientState.Pitch = -89;
}

void client::shutDown() {
}

client::~client() {
}

void client::updatePosition() {
  glm::vec3 Direction;
  Direction.x = cos(glm::radians(ClientState.Pitch)) * cos(glm::radians(ClientState.Yaw));
  Direction.y = sin(glm::radians(ClientState.Pitch));
  Direction.z = cos(glm::radians(ClientState.Pitch)) * sin(glm::radians(ClientState.Yaw));
  CameraData.Front = glm::normalize(Direction);

  float CameraSpeed = 0.05;// * WindowData.DeltaTime;
  //Else-if because you can't move frowards and backwards simoultaneously!
  if (ClientState.Keyboard[0])
    CameraData.Position += CameraData.Front * CameraSpeed;
  else if (ClientState.Keyboard[1])
    CameraData.Position -= CameraData.Front * CameraSpeed;
  if (ClientState.Keyboard[2])
    CameraData.Position -= glm::normalize(glm::cross(CameraData.Front, CameraData.Up)) * CameraSpeed;
  else if (ClientState.Keyboard[3])
    CameraData.Position += glm::normalize(glm::cross(CameraData.Front, CameraData.Up)) * CameraSpeed;
  if (ClientState.Keyboard[4])
    CameraData.Position += CameraData.Up * CameraSpeed;
  else if (ClientState.Keyboard[5])
    CameraData.Position -= CameraData.Up * CameraSpeed;
}
