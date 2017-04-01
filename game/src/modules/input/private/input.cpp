#include "input.h"

input::input() {
}

void input::start() {
  setName("Input", this);
  addFunction("updateEvents", &input::updateEvents);
}

void input::updateEvents(SDL_Event Event) {
  switch (Event.type) {
    case SDL_QUIT : request("Window", "quit");
                    break;
    case SDL_KEYDOWN : keyboardEvent(Event.key);
                       break;
    case SDL_KEYUP : keyboardEvent(Event.key);
                     break;
    case SDL_MOUSEMOTION : request("Client", "setMouse", Event.motion.x, Event.motion.y);
  }
}

void input::shutDown() {
}

input::~input() {
}

void input::keyboardEvent(SDL_KeyboardEvent Keyboard) {
  bool State;
  if (Keyboard.state == SDL_PRESSED)
    State = true;
  else State = false;
  if (Keyboard.keysym.sym > 1024)
    Keyboard.keysym.sym = 1024;
  if (KeyPressedState[Keyboard.keysym.sym] != State) {
    switch (Keyboard.keysym.sym) {
      case SDLK_w : request("Client", "setState", std::string("Forward"), State);
               break;
      case SDLK_s : request("Client", "setState", std::string("Backward"), State);
               break;
      case SDLK_a : request("Client", "setState", std::string("Left"), State);
               break;
      case SDLK_d : request("Client", "setState", std::string("Right"), State);
    }
    KeyPressedState[Keyboard.keysym.sym] = State;
  }
}
