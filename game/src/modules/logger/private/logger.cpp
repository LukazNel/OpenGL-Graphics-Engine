#include "logger.h"

logger::logger() {
  LogOut.open("log");
}

void logger::start() {
  setName("Logger", this);
  addFunction("log", &logger::log);
}

void logger::log(std::string Module, std::string String) {
  std::cout << std::flush << Module << " says: " << String << std::endl;
  //std::this_thread::sleep_for(std::chrono::seconds(1));
  //for (int i = 0; i < 1000; i++) {
    //continue;
  //}
}

void logger::shutDown() {}
