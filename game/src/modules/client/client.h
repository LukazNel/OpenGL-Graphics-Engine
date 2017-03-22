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
  void setCameraPointers(float* CSMatrix, float* WSMatrix, float* Position, std::atomic<bool>* DataIsReady);
  void update();
  void setState(std::string Type, bool State);
  virtual void shutDown();
  virtual ~client();
private:
  struct rendererstruct {
    float* CSMatrix;
    float* WSMatrix;
    float* Position;
    std::atomic<bool>* DataIsReady;
  };

  rendererstruct RendererData;
  glm::vec3 CameraPosition;
};

#endif // _GAME_ENGINE_SRC_MODULE_CLIENT_H_
