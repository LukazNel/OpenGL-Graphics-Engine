#include <memory>
#include <string>

#include "manager.h"
#include "modules/module.h"
#include "modules/logger/logger.h"
#include "modules/window/window.h"
#include "modules/renderer/renderer.h"

int main() {
  manager Manager;
  std::unique_ptr<module> Logger(new logger);
  std::unique_ptr<module> Window(new window);
  std::unique_ptr<module> Renderer(new renderer);
  Manager.addModule(Logger);
  Manager.addModule(Window);
  Manager.addModule(Renderer);
  Manager.request("Window", "begin", true);
  Manager.request("Renderer", "prepare", true);
  Manager.start();
  Manager.shutDown();
  return 0;
}
