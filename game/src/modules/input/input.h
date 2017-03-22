#ifndef _GAME_ENGINE_SRC_MODULE_INPUT_H_
#define _GAME_ENGINE_SRC_MODULE_INPUT_H_

#include <memory>
#include <atomic>

#include "modules/module.h"
#include "external/include/SDL2/SDL_events.h"

class input : public module {
 public:
  input();
  virtual void start();
  void updateEvents(SDL_Event Event);
  virtual void shutDown();
  virtual ~input();
 private:
  void keyboardEvent(SDL_KeyboardEvent Keyboard);

  bool KeyPressedState[1024];
};

#endif // _GAME_ENGINE_SRC_MODULE_INPUT_H_
