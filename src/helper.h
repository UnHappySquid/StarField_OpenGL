#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <random>

// Shaders 
//  Vertex Shader (performs projection)
const std::string vs = R"glsl(
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color; 
out vec4 c_in; 

uniform float zNear;
uniform float zFar;
uniform float frustumScale;
uniform float aspect;

void main()
{
   vec4 cameraPos = position;
   vec4 clipPos;
   
   clipPos.xy = cameraPos.xy * frustumScale;
   clipPos.x /= aspect;
   clipPos.z = cameraPos.z * zFar / (zFar - zNear);
   clipPos.z -= zNear * zFar / (zFar - zNear);
   clipPos.w = cameraPos.z;
    
   gl_Position = clipPos;
   c_in = color; 
}
)glsl";

//  fragment Shader 
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
const double sens_x = 0.01;
const double sens_y = 0.01;
const double FPS = 60, frame_duration = 1 / FPS;
const float Zfar = 5;
const float Znear = 0.001;

// Possible variables but probably constants as well
int WIDTH, HEIGHT;
float aspect;

// variables
float FOV = 90;
float f = 1. / tanf((FOV * PI / 180) / 2);

// Random generators
const int seed = 11234212512332233;
std::default_random_engine eng(seed);
std::uniform_real_distribution<float> dis_xy(-2.5f, 2.5f);
std::uniform_real_distribution<float> dis_xy_far(1.f, 2.5f);
std::uniform_real_distribution<float> dis_z(-5.f, 5.f);
std::uniform_real_distribution<float> dis_z_far(1.f, 5.f);
std::uniform_real_distribution<float> dis_color(0.f, 1.f);

// Used as 3d position with w for good mesure
struct Position {
    float x, y, z, w;
};

// Operator overloads for printing and what not
std::ostream& operator<<(std::ostream& os, Position& p) {
    os << "{ " << p.x << ", " << p.y << ", " << p.z << ", " << p.w << " }";
    return os;
}
bool operator==(Position& p1, Position& p2) {
    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}
bool operator!=(Position& p1, Position& p2) {
    return !(p1 == p2);
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

// initiate window
GLFWwindow* window_init() {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        throw std::runtime_error("Couldn't initialize glfw");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    
    const GLFWvidmode* return_struct = glfwGetVideoMode(mon);

    WIDTH = return_struct->width;
    HEIGHT = return_struct->height;
    aspect = float(WIDTH) / HEIGHT;
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WIDTH, HEIGHT, "Hello World", glfwGetPrimaryMonitor(), NULL);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Couldnt create window");
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);



    return window;
}

// Load open_GL
void load_OpenGL() {
    // Load openGL
    int version = gladLoadGL();
    if (version == 0) {
        throw std::runtime_error("Failed to initialize OpenGL context\n");
    }
}

// Creates program and links shaders
static unsigned int create_and_use_shaders(const std::string& vertexShader, const std::string& fragmentShader) {
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

    return id;
}

// frees resources related to glfw
void terminate(GLFWwindow* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Generates float between -2.5 and 2.5
float generate_xy() {
    return dis_xy(eng);
}

// Generates float between 1 and 2.5
float generate_xy_far() {
    return dis_xy_far(eng);
}

// Generates float between -5 and 5
float generate_z() {
    return dis_z(eng);
}

// Generates float between 1 and 5
float generate_z_far() {
    return dis_z_far(eng);
}

// Generates a value between 0 and 1 for color use
float generate_color() {
    return dis_color(eng);
}

// helper
float to_rad(float deg) {
    return deg * PI / 180;
}

template <int N>
class Circle {
    unsigned int va, vb, eb;

    float radius;
    Position center;
    float red, green, blue;

    Vertex vertices[N];
    Position pos[N];
    unsigned int indices[(N - 2) * 3];

    void gen_circle(Position center, float r) {
        float angle = 360.0f / N;


        for (int i = 0; i < N; i++)
        {
            float currentAngle = angle * i;
            pos[i].x = center.x + radius * cos(currentAngle * PI / 180);
            pos[i].y = center.y + radius * sin(currentAngle * PI / 180);
            pos[i].z = center.z;
            pos[i].w = center.w;

        }

    }

    void fill_vertices() {
        for (size_t i = 0; i < N; i++)
        {
            vertices[i].p = pos[i];
            vertices[i].r = red;
            vertices[i].g = green;
            vertices[i].b = blue;
            vertices[i].a = 1.f;
        }
    }

public:

    Circle(Position center, float r, float red, float green, float blue) : center{ center }, radius{ r }
        , red{ red }, green{ green }, blue{blue} {
        glGenVertexArrays(1, &va);
        glGenBuffers(1, &vb);
        glGenBuffers(1, &eb);

        glBindVertexArray(va);
        glBindBuffer(GL_ARRAY_BUFFER, vb);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb);
        gen_circle(center, r);


        int triangleCount = N - 2;

        // triangles
        for (int i = 0; i < triangleCount; i++)
        {
            indices[i * 3] = 0;
            indices[i * 3 + 1] = i + 1;
            indices[i * 3 + 2] = i + 2;
        }

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

        //  Telling open gl how to interpret vertices and enable vertex attrib
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void draw() {
        if (center.z >= 0) {
            glBindVertexArray(va);
            glBindBuffer(GL_ARRAY_BUFFER, vb);
            gen_circle(center, radius);
            fill_vertices();
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);

            glDrawElements(GL_TRIANGLES, (N - 2) * 3, GL_UNSIGNED_INT, 0);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
    }

    void move_all_by(float dx, float dy, float dz) {
        center.x += dx; center.y += dy, center.z += dz;
    }

    void move_all_to(Position p) {
        center = p;
    }

    void resize(float dr) {
        if (radius + dr > 0)
        radius += dr;
    }

    Position get_center() {
        return center;
    }

    unsigned int get_va() {
        return va;
    }
};