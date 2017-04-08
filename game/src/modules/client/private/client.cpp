#include "client.h"

client::client() {
  CameraData.WSMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.0625)); // or 0.03125
  CameraData.Position = {0, 10, 0};
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

void client::setCameraPointers(float* CSMatrix, float* WSMatrix, float* SkydomeMatrix, float* Position, std::atomic<bool>* DataIsReady) {
  RendererData.CSMatrix = CSMatrix;
  RendererData.WSMatrix = WSMatrix;
  RendererData.SkydomeMatrix = SkydomeMatrix;
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
  if (CameraData.Position.y < 2)
    CameraData.Position.y = 2;
  CameraData.CameraMatrix = glm::lookAt(CameraData.Position, CameraData.Position + CameraData.Front, CameraData.Up);
  CameraData.SkydomeMatrix = CameraData.PerspectiveMatrix * glm::lookAt(glm::vec3(CameraData.Position.x, CameraData.Position.y + 0.0, CameraData.Position.z), CameraData.Position + glm::vec3(CameraData.Front.x, -CameraData.Front.y, -CameraData.Front.z), CameraData.Up);
  
  //if (anything changes)
  //  WSMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.03125));
  
  CameraData.CSMatrix = CameraData.PerspectiveMatrix * CameraData.CameraMatrix;

  if (RendererData.DataIsReady->load() == false) {
    std::copy(glm::value_ptr(CameraData.CSMatrix), glm::value_ptr(CameraData.CSMatrix) + 16, RendererData.CSMatrix);
    std::copy(glm::value_ptr(CameraData.WSMatrix), glm::value_ptr(CameraData.WSMatrix) + 16, RendererData.WSMatrix);
    std::copy(glm::value_ptr(CameraData.SkydomeMatrix), glm::value_ptr(CameraData.SkydomeMatrix) + 16, RendererData.SkydomeMatrix);
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

  int Limit = 80;
  if (ClientState.Pitch > Limit)
    ClientState.Pitch = Limit;
  else if (ClientState.Pitch < -Limit)
    ClientState.Pitch = -Limit;
}

void client::shutDown() {
}

client::~client() {
}

void client::updatePosition() {
  glm::vec3 Front;
  Front.x = cos(glm::radians(ClientState.Pitch)) * cos(glm::radians(ClientState.Yaw));
  Front.y = sin(glm::radians(ClientState.Pitch));
  Front.z = cos(glm::radians(ClientState.Pitch)) * sin(glm::radians(ClientState.Yaw));
  CameraData.Front = glm::normalize(Front);

  float CameraSpeed = 0.5; //* WindowData.DeltaTime;
  //Else-if because you can't move frowards and backwards simoultaneously!
  if (ClientState.Keyboard[0])
    CameraData.Position += glm::vec3(CameraData.Front.x * CameraSpeed, 0, CameraData.Front.z * CameraSpeed);
  else if (ClientState.Keyboard[1])
    CameraData.Position -= glm::vec3(CameraData.Front.x * CameraSpeed, 0, CameraData.Front.z * CameraSpeed);
  if (ClientState.Keyboard[2])
    CameraData.Position -= glm::normalize(glm::cross(CameraData.Front, CameraData.Up)) * (CameraSpeed / 2);
  else if (ClientState.Keyboard[3])
    CameraData.Position += glm::normalize(glm::cross(CameraData.Front, CameraData.Up)) * (CameraSpeed / 2);
  if (ClientState.Keyboard[4])
    CameraData.Position += CameraData.Up * CameraSpeed;
  else if (ClientState.Keyboard[5])
    CameraData.Position -= CameraData.Up * CameraSpeed;
}
