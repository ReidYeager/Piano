
#include "defines.h"
#include "window.h"
#include "shader.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <math.h>

void HandleInput(GLFWwindow* _window)
{
  if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(_window, true);
}

GLFWwindow* Initialize()
{
  // Initialize GLFW
  GLFWwindow* window = InitializeWindow(800, 600, "Piano");

  // Initialize GLAD / OpenGL
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    printf("Failed to initialize GLAD\n");
    throw(-1);
  }

  glViewport(0, 0, 800, 600);

  GetShaderProgram({"base.vert", "base.frag"}, {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER});

  return window;
}

glm::mat4x4 mvp;

void Render()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  for (const auto& m : materials)
  {
    glUseProgram(m.shaderProgram);
    u32 matid = glGetUniformLocation(m.shaderProgram, "mvp");
    glUniformMatrix4fv(matid, 1, GL_FALSE, glm::value_ptr(mvp));
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }
}

float pianoTime = 0.0f;

void Loop(GLFWwindow* _window)
{
  while (!glfwWindowShouldClose(_window))
  {
    HandleInput(_window);

    pianoTime += 0.01f;

    mvp = glm::translate(glm::mat4(1), glm::vec3(std::sin(pianoTime), 0.0f, 0.0f));

    Render();
    glfwSwapBuffers(_window);
    glfwPollEvents();
  }
}

int main()
{
  try {
    GLFWwindow* window = Initialize();
    Loop(window);
  }
  catch (int e)
  {
    glfwTerminate();
    return e;
  }

  glfwTerminate();
  return 0;
}

