#ifndef _GAME_ENGINE_SRC_MODULE_CLIENT_H_
#define _GAME_ENGINE_SRC_MODULE_CLIENT_H_

#include <memory>
#include <atomic>
#include <string>

#include "modules/module.h"

#include "external/include/glm/glm.hpp"
#include "external/include/glm/gtc/type_ptr.hpp"
#include "external/include/glm/gtc/matrix_transform.hpp"

class client : public module {
 public:
  client();
  virtual void start();
  void setCameraPointers(float* CSMatrix, float* WSMatrix, float* SkyboxMatrix, float* Position, std::atomic<bool>* DataIsReady);
  void update();
  void setState(std::string Type, bool State);
  void setMouse(int MouseX, int MouseY);
  virtual void shutDown();
  virtual ~client();
private:
  struct rendererstruct {
    float* CSMatrix;
    float* WSMatrix;
    float* SkyboxMatrix;
    float* Position;
    std::atomic<bool>* DataIsReady;
  };
  struct camerastruct {
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::mat4 PerspectiveMatrix;
    glm::mat4 CameraMatrix;
    glm::mat4 WSMatrix;
    glm::mat4 CSMatrix;
    glm::mat4 SkyboxMatrix;
  };
  struct windowstruct {
    int WindowWidth;
    int WindowHeight;
    float DeltaTime;
  };
  struct statestruct {
    bool Keyboard[6];
    int LastMouseX;
    int LastMouseY;
    float Pitch;
    float Yaw;
  };

  void updatePosition();

  rendererstruct RendererData;
  camerastruct CameraData;
  windowstruct WindowData;
  statestruct ClientState;
};

#endif // _GAME_ENGINE_SRC_MODULE_CLIENT_H_
