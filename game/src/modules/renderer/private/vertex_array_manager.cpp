#include "classes/vertex_array_manager.h"

vertexarraymanager::vertexarraymanager() {
  LogString += '\n';
}

void vertexarraymanager::createVertexArray(const std::string VertexArrayName, int Location, int AttributeSize, GLenum AttributeType, bool Normalize, int Stride, const GLvoid* StartPosition) {
  GLuint VertexArrayVar;
  glGenVertexArrays(1, &VertexArrayVar);
  glBindVertexArray(VertexArrayVar);
  glVertexAttribPointer(Location, AttributeSize, AttributeType, Normalize, Stride, StartPosition);
  vertexarraystruct VertexArrayStructVar;
  VertexArrayStructVar.VertexArrayName = VertexArrayName;
  VertexArrayStructVar.VertexArrayLocation = Location;
  VertexArrayStructVar.VertexArrayHandle = VertexArrayVar;
  VertexArrayArray.push_back(VertexArrayStructVar);
  LogString += "Vertex array '" + VertexArrayName + "' successfully created.\n";
}

void vertexarraymanager::installVertexArray(const std::string VertexArrayName) {
  bool VertexArrayFound;
  auto VertexArrayIterator = findVertexArray(VertexArrayName, VertexArrayFound);
  if (VertexArrayFound) {
    glEnableVertexAttribArray(VertexArrayIterator->VertexArrayLocation);
    LogString += "Vertex Array '" + VertexArrayName + "' enabled.\n";
  }
}

std::string vertexarraymanager::getLog() {
  std::string Log = LogString;
  LogString = '\n';
  return Log;
}

void vertexarraymanager::cleanUp() {
  for (auto VertexArrayIterator : VertexArrayArray) {
    glDeleteVertexArrays(1, &VertexArrayIterator.VertexArrayHandle);
  }
}

vertexarraymanager::~vertexarraymanager() {
}

auto vertexarraymanager::findVertexArray(const std::string VertexArrayName, bool& VertexArrayFound) -> std::vector<vertexarraystruct>::iterator {
  auto VertexArrayIterator = std::find_if(VertexArrayArray.begin(), VertexArrayArray.end(), 
      [&] (const vertexarraystruct& VertexArrayStructVar) {return VertexArrayStructVar.VertexArrayName == VertexArrayName;});
  if (VertexArrayIterator != VertexArrayArray.end()) {
    VertexArrayFound = true;
    return VertexArrayIterator;
  } else {
    VertexArrayFound = false;
    LogString = "Warning: vertex array '" + VertexArrayName + "' not found.\n";
    return VertexArrayIterator;
  }
}
