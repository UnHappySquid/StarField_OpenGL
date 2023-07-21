#include "helper.h"

int main() {
	GLFWwindow* window = window_init();
	load_OpenGL();
	create_and_use_shaders(vs, fs);

	// Set up here
	double t1 = 0;
	double t2 = glfwGetTime();
	glClearColor(0.1, 0.1, 0.1, 1);
	// Game Loop
    while (!glfwWindowShouldClose(window))
    {
        if (t2 - t1 >= refresh_rate) {
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window);
            glfwPollEvents();
            t1 = t2;
        }

        t2 = glfwGetTime();
    }

	terminate(window);
}