#include <memory>
#include <string>

#include "manager.h"
#include "modules/module.h"
#include "modules/logger/logger.h"
#include "modules/window/window.h"
#include "modules/renderer/renderer.h"
#include "modules/input/input.h"
#include "modules/client/client.h"

int main() {
  manager Manager;
  std::unique_ptr<module> Logger(new logger);
  std::unique_ptr<module> Window(new window);
  std::unique_ptr<module> Renderer(new renderer);
  std::unique_ptr<module> Input(new input);
  std::unique_ptr<module> Client(new client);
  Manager.addModule(Logger);
  Manager.addModule(Window);
  Manager.addModule(Renderer);
  Manager.addModule(Input);
  Manager.addModule(Client);

  Manager.request("Window", "begin", true);
  Manager.request("Renderer", "prepare", true);
  Manager.start();
  Manager.shutDown();
  return 0;
}

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nShowCmd) {
    return main();
}
#endif
