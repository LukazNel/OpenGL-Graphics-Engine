#include <cmath>
#include <string>
#include <ostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <src/modules/view_module/classes/context.h>
#include "code/RenderingComponent.h"
#include "colourArray.h"


#define DEBUG

static std::ostream logger();

static const GLchar vertexShaderSource[] = {
  "#version 420 core                                                 \n"
  "in vec4 position;                                                 \n"
  "void main(void)                                                   \n"
  "{                                                                 \n"
  "    gl_Position = position;     \n"
  "}                                                                 \n"
};

static const GLchar geometryShaderSource[] = {
          "#version 420 core\n"
          "layout(points) in;\n"
          "layout(triangle_strip) out;\n"
          "layout (max_vertices = 9) out;\n"
          "out vec4 icolour;\n"
          "uniform vec4 centerPoint;\n"
          "uniform mat4 projection;\n"
          "layout(std140) uniform colourArray {\n"
                  "vec4 colours[1022];\n"
          "};\n"
          "void main(void) {\n"
          "     vec4 uniColour = colours[int(gl_in[0].gl_Position.w)];"
          "     gl_Position = projection * vec4(gl_in[0].gl_Position.x - centerPoint.x, gl_in[0].gl_Position.y + 0.0f, gl_in[0].gl_Position.z + 0.0f, 1.0f);\n"
          "     icolour = uniColour;\n"
          "     EmitVertex();\n"
          "     gl_Position = projection * vec4(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y, gl_in[0].gl_Position.z, 1.0f);\n"
          "     icolour = uniColour.xyzw;\n"
          "     EmitVertex();\n"
          "     gl_Position = projection * vec4(gl_in[0].gl_Position.x - centerPoint.x, gl_in[0].gl_Position.y - centerPoint.y, gl_in[0].gl_Position.z + 0.0f, 1.0f);\n"
          "     icolour = uniColour.xyzw;\n"
          "     EmitVertex();\n"
          "     gl_Position = projection * vec4(gl_in[0].gl_Position.x  + 0.0f, gl_in[0].gl_Position.y - centerPoint.y, gl_in[0].gl_Position.z + 0.0f, 1.0f);\n"
          "     icolour = uniColour.xyzw;\n"
          "     EmitVertex();\n"
          "     gl_Position = projection * vec4(gl_in[0].gl_Position.x + 0.0f, gl_in[0].gl_Position.y - centerPoint.y, gl_in[0].gl_Position.z - centerPoint.z, 1.0f);\n"
          "     icolour = uniColour.xyzw;\n"
          "     EmitVertex();\n"
          "     gl_Position = projection * vec4(gl_in[0].gl_Position.x + 0.0f, gl_in[0].gl_Position.y + 0.0f, gl_in[0].gl_Position.z + 0.0f, 1.0f);\n"
          "     icolour = uniColour.xyzw;\n"
          "     EmitVertex();\n"
          "     gl_Position = projection * vec4(gl_in[0].gl_Position.x + 0.0f, gl_in[0].gl_Position.y + 0.0f, gl_in[0].gl_Position.z - centerPoint.z, 1.0f);\n"
          "     icolour = uniColour.xyzw;\n"
          "     EmitVertex();\n"
          "     gl_Position = projection * vec4(gl_in[0].gl_Position.x - centerPoint.x, gl_in[0].gl_Position.y + 0.0f, gl_in[0].gl_Position.z + 0.0f, 1.0f);\n"
          "     icolour = uniColour.xyzw;\n"
          "     EmitVertex();\n"
          "     gl_Position = projection * vec4(gl_in[0].gl_Position.x - centerPoint.x, gl_in[0].gl_Position.y + 0.0f, gl_in[0].gl_Position.z - centerPoint.z, 1.0f);\n"
          "     icolour = uniColour.xyzw;\n"
          "     EmitVertex();\n"
          "     EndPrimitive;\n"
          "}"
};

static const GLchar fragmentShaderSource[] = {
  "#version 420 core                                                 \n"
  "in vec4 icolour;                                                  \n"
  "out vec4 color;                                                   \n"
// "uniform mat4 projection\n"
  "void main(void)                                                   \n"
  "{                                                                 \n"
// "    float CameraFacingPercentage = dot(proj)"
  "    color = icolour;                             \n"
  "}                                                                 \n"
};

static GLfloat indices[] = {
          0.0f, 1.0f, -1.0f, 2.0f,
          0.0f, 1.0f, 0.0f, 32.0f,
          0.0f, 1.0f, 1.0f, 32.0f,
          0.0f, 1.0f, 2.0f, 2.0f,
          0.0f, 2.0f, -1.0f, 45.0f
};

void changeIndices(void) {
  if (indices[1] >= 3.0f)
          indices[1] -= 1.0f;
  else if (indices[1] <= -3.0f)
          indices[1] += 1.0f;
  }

int main() {
  contextManager context;
  context.start();
// cout ;
  RenderingComponent renderer;

  GLuint bindingPoint = 1, colourBuffer;
  GLint cUniform = glGetUniformBlockIndex(program, "colourArray");

  glUniformBlockBinding(program, cUniform, bindingPoint);
  glGenBuffers(1, &colourBuffer);
  glBindBuffer(GL_UNIFORM_BUFFER, colourBuffer);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(colourArray), colourArray, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, colourBuffer);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  GLint pUniform = glGetUniformLocation(program, "projection");
  GLint pointUniform = glGetUniformLocation(program, "centerPoint");

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  int increment = 0;
  int points = sizeof(indices)/(sizeof(GLfloat)*4);
  while (!context.ifQuit()) {
    if (increment == 60) {
      changeIndices();
      increment = 0;
    }
  glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);
    GLfloat camY = sin(glfwGetTime());
    glm::mat4 perspective = glm::perspective(glm::radians(135.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(2, 3, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
    glm::mat4 projectionMatrix = perspective * view * model;
    projectionMatrix = glm::rotate(projectionMatrix, glm::mediump_float(glfwGetTime()), glm::vec3(0, 1, 0));
    context.frameCount();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnableVertexAttribArray(0);

    glUniformMatrix4fv(pUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniform4f(pointUniform, 1.0f, 1.0f, -1.0f, 1.0f);
    glDrawArrays(GL_POINTS, 0, points);
    context.refresh();

    increment++;
  }
  glDeleteProgram(program);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  context.shutdown();
  return 0;
}
