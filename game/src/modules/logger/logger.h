#include <string>
#include <memory>
#include <fstream>
#include <iostream>

#include "modules/module.h"

class logger : public module {
 public:
  logger();
  virtual void start();
  void log(std::string, std::string);
  virtual void shutDown();

 private:
  std::ofstream LogOut;
};
