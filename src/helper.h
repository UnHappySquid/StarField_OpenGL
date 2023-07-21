#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath>

// Shaders
const std::string vs = R"glsl(
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color; 
out vec4 c_in; 

void main(){
   c_in = color; 
   gl_Position = position;
}
)glsl";

const std::string fs = R"glsl(
#version 330 core
in vec4 c_in; 
out vec4 color;

void main(){
    color = c_in;
}
)glsl";

// Constants
const double PI = 3.14159265359;
const int WIDTH = 480, HEIGHT = 480;
const double FPS = 144, refresh_rate = 1 / FPS;
const float FOV = 90;
const float Zfar = 10000;
const float Znear = 0.1;
const float aspect = float(WIDTH) / float(HEIGHT);
const float f = 1. / tanf((FOV * PI / 180) / 2);
const float q = Zfar / (Zfar - Znear);

// Used as 3d position with w for good mesure
struct Position {
    float x, y, z, w;
};

std::ostream& operator<<(std::ostream& os, Position& p) {
    os << "{ " << p.x << ", " << p.y << ", " << p.z << ", " << p.w << " }";
    return os;
}

// Used for data transfer to GPU
struct Vertex {
    Position p;
    float r, g, b, a;
};

// Shader compiler
static unsigned int compile_shader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        char* message = (char*)alloca(len * sizeof(char));
        glGetShaderInfoLog(id, len, &len, message);
        std::cout << "Failed to compile shader" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

GLFWwindow* window_init() {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        throw std::runtime_error("Couldn't initialize glfw");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WIDTH, HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Couldnt create window");
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    return window;
}

void load_OpenGL() {
    // Load openGL
    int version = gladLoadGL();
    if (version == 0) {
        throw std::runtime_error("Failed to initialize OpenGL context\n");
    }
}

// Creates program and links shaders
static void create_and_use_shaders(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int id = glCreateProgram();
    unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(id, vs);
    glAttachShader(id, fs);

    glLinkProgram(id);
    glValidateProgram(id);

    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(id);
}

void terminate(GLFWwindow* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}