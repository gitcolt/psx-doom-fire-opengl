#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

const unsigned WIDTH = 800;
const unsigned HEIGHT = 600;

const int IMAGE_WIDTH = 200;
const int IMAGE_HEIGHT = 200;
const int CHANNEL_COUNT = 4;
const int IMAGE_DATA_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT * CHANNEL_COUNT;
const GLenum PIXEL_FORMAT = GL_BGRA;

#define PALLETE_SIZE 36

static const int pallete[PALLETE_SIZE] = {
  0x00070707,
  0x011f0707,
  0x022f0707,
  0x03470f07,
  0x04571707,
  0x05671f07,
  0x06771f07,
  0x078f2707,
  0x089f2f07,
  0x09af3f07,
  0x0abf4707,
  0x0bc74707,
  0x0cdf4f07,
  0x0ddf5707,
  0x0edf5707,
  0x0fd75f07,
  0x10d7670f,
  0x11cf6f0f,
  0x12cf770f,
  0x13cf7f0f,
  0x14cf8717,
  0x15c78717,
  0x16c78f17,
  0x17c7971f,
  0x18bf9f1f,
  0x19bf9f1f,
  0x1abfa727,
  0x1bbfa727,
  0x1cbfaf2f,
  0x1db7af2f,
  0x1eb7b72f,
  0x1fb7b737,
  0x20cfcf6f,
  0x21dfdf9f,
  0x22efefc7,
  0x23ffffff,
};

void spread_fire(int32_t *fire, int from) {
  int alpha = fire[from] >> 24;
  int r = rand() % 3;

  int to = from + IMAGE_WIDTH + r - 1;

  fire[to] = pallete[alpha - (r & 1)];
}

void update_pixels(int32_t *dst) {
  for (int y = 0; y < IMAGE_HEIGHT; y++) {
    for (int x = 0; x < IMAGE_WIDTH; x++) {
      spread_fire(dst, (y * IMAGE_WIDTH) + x);
    }
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

unsigned set_up_shader(const char *source_path, unsigned shader_type) {
  FILE *fp = fopen(source_path, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to read '%s'", source_path);
  }
  fseek(fp, 0, SEEK_END);
  long source_size = ftell(fp);
  char *buf = malloc(source_size + 1);
  fseek(fp, 0, SEEK_SET);
  fread(buf, source_size, 1, fp);
  fclose(fp);
  buf[source_size] = '\0';

  unsigned shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, (const char * const *)&buf, NULL);
  glCompileShader(shader);
  int success;
  char info_log[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, info_log);
    fprintf(stderr, "Shader compilation failed\n%s\n", info_log);
  }

  return shader;
}

int main() {
  srand(time(NULL));

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }

  unsigned vertex_shader = set_up_shader("shader/vert.glsl", GL_VERTEX_SHADER);
  unsigned fragment_shader = set_up_shader("shader/frag.glsl", GL_FRAGMENT_SHADER);

  unsigned shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  int success;
  char info_log[512];
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, info_log);
    fprintf(stderr, "Shader program compilation failed\n%s\n", info_log);
    return -1;
  }
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  float vertices[] = {
    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
     1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
  };

  unsigned indices[] = {
    0, 1, 2,
    2, 1, 3,
  };

  unsigned VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  unsigned texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  int32_t *image_data = malloc(IMAGE_DATA_SIZE);

  for (int col = 0; col < IMAGE_HEIGHT; col++) {
    for (int row = 0; row < IMAGE_WIDTH; row++) {
      int i = (col == 0) ? PALLETE_SIZE - 1 : 0;
      image_data[col * IMAGE_WIDTH + row] = pallete[i];
    }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, IMAGE_WIDTH, IMAGE_HEIGHT, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid*)image_data);

  glVertexAttribPointer(
      1, // index
      2, // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      5 * sizeof(float), // stride
      (void *)(3 * sizeof(float)) // offset
      );
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  unsigned PBO;
  glGenBuffers(1, &PBO);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, IMAGE_DATA_SIZE, 0 /* NULL*/, GL_STREAM_DRAW);

  glBindVertexArray(0);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glUseProgram(shader_program);
    glBindVertexArray(VAO);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO);
    // Do I have to do this?
    glBufferData(GL_PIXEL_UNPACK_BUFFER, IMAGE_DATA_SIZE, 0, GL_STREAM_DRAW);

    int32_t *ptr = (int32_t*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE);

    if (ptr) {
      // why is ptr sometimes a wrong value?
      if (*ptr == pallete[PALLETE_SIZE - 1]) {
        update_pixels(ptr);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
      }
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glClearColor(0.4f, 0.4f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shader_program);
  glfwTerminate();
  return 0;
}
