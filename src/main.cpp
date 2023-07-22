#include "helper.h"

using namespace std;

const int num_stars = 1000;
const float star_size = 0.001;
Position speed = { 0, 0, 0, 1 };
const float max_speed_z = 0.1;
const float max_speed = 5;
const float min_speed = -5;

// Circle holds the OpenGL implementation details to get a cirle on the screen
// Star holds that and the implementation to get a line behind the star
class Star : protected Circle<32>{
    Position old_center;
public:
    Star(Position center) : Circle{center, star_size}, old_center{ center } {
    }

    void bound_check_logic() {
        Position p = get_center();
        Position new_center = p;

        if (p.x <= -2.f || p.x >= 2.f) {
            new_center.x = generate_xy();
        }
        if (p.y <= -2.f || p.y >= 2.f) {
            new_center.y = generate_xy();

        }
        if (p.z < 0.f || p.z > 5.f) {
            new_center.z = p.z < 0.f ? generate_z_far() : 0.f;
        }
        old_center = new_center;
        move_all_to(new_center);
    }
    void tick(float dt, Position velocity) {
        bound_check_logic();
        move_all_by(velocity.x * dt, velocity.y * dt, velocity.z * dt);
        draw();
    }

    void rotate_z(float deg) {
        Position new_c;
        Position curr_c = get_center();
        new_c.x = curr_c.x * cos(to_rad(deg)) 
            - curr_c.y * sin(to_rad(deg));
        new_c.y = curr_c.x * sin(to_rad(deg)) + curr_c.y * cos(to_rad(deg));
        new_c.z = curr_c.z;
        new_c.w = curr_c.w;
        move_all_to(new_c);
    }

    void rotate_x(float deg) {
        Position new_c;
        Position curr_c = get_center();
        new_c.y = curr_c.y * cos(to_rad(deg))
            - curr_c.z * sin(to_rad(deg));
        new_c.z = curr_c.y * sin(to_rad(deg)) + curr_c.z * cos(to_rad(deg));
        new_c.x = curr_c.x;
        new_c.w = curr_c.w;
        move_all_to(new_c);
    }

    void rotate_y(float deg) {
        Position new_c;
        Position curr_c = get_center();
        new_c.x = curr_c.x * cos(to_rad(deg))
            + curr_c.z * sin(to_rad(deg));
        new_c.z = - curr_c.x * sin(to_rad(deg)) + curr_c.z * cos(to_rad(deg));
        new_c.y = curr_c.y;
        new_c.w = curr_c.w;
        move_all_to(new_c);
    }
};

// Generates the stars and holds main way to make movement
class Starfield {
    vector<Star*> stars;
    Position field_velocity;
public:
    Starfield(int num_stars, Position vel) : field_velocity{ vel } {

        for (size_t i = 0; i < num_stars; i++)
        {
            // Generate random position in x, y, z
            Position ran_pos{ generate_xy(),
            generate_xy(),
            generate_z_normalized(),
            1 };

            // Make new star
            Star* star = new Star(ran_pos);
            // Push back into star vector 
            stars.push_back(star);
        }
    }
    void tick(float dt) {
        for (Star* s : stars) {
            s->tick(dt, field_velocity);
        }
    }


    void rotate_around_x(float deg) {
        for (Star* s : stars) {
            s->rotate_x(deg);
        }
    }
    void rotate_around_y(float deg) {
        for (Star* s : stars) {
            s->rotate_y(deg);
        }
    }
    void rotate_around_z(float deg) {
        for (Star* s : stars) {
            s->rotate_z(deg);
        }
    }


    void set_velocity(Position vel) {
        field_velocity = vel;
    }

    Position get_velocity() {
        return field_velocity;
    }

    Position& get_velocity_ref() {
        return field_velocity;
    }
};

void process_input(GLFWwindow* window, Starfield& field) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
        field.get_velocity().z >= min_speed) {
        field.get_velocity_ref().z -= 0.01;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS 
        && field.get_velocity().z <= max_speed_z) {
        field.get_velocity_ref().z += 0.01;
    }
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        field.get_velocity_ref() = {0, 0, 0, 1};
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS
        && field.get_velocity().x <= max_speed) {
        field.get_velocity_ref().x += 0.01;

    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS
        && field.get_velocity().x >= min_speed) {
        field.get_velocity_ref().x -= 0.01;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS
        && field.get_velocity().y <= max_speed) {
        field.get_velocity_ref().y += 0.01;

    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
        && field.get_velocity().y >= min_speed) {
        field.get_velocity_ref().y -= 0.01;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        field.rotate_around_z(0.5);
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {

        field.rotate_around_z(-0.5);
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }


    // This is all back in the yucky pixel coordinate system
    double dx, dy;
    glfwGetCursorPos(window, &dx, &dy);

    dx = (WIDTH / 2) - dx;
    dy = (HEIGHT / 2) - dy;
    field.rotate_around_y(dx * sens_x);
    field.rotate_around_x(dy * sens_y);
}

int main() {
	GLFWwindow* window = window_init();
	load_OpenGL();
	create_and_use_shaders(vs, fs);



	// Set up here --------------------------------------------
	double t1 = 0;
	double t2 = glfwGetTime();
    //  Background color
	glClearColor(0.1, 0.1, 0.1, 1);
	// StarField
    Starfield field{num_stars, speed};
    // -------------------------------------------------------
    
    // Game Loop ---------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        if (t2 - t1 >= frame_duration) {
            

            glClear(GL_COLOR_BUFFER_BIT);
            field.tick(frame_duration);
            process_input(window, field);

            glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);

            glfwSwapBuffers(window);
            glfwPollEvents();
            t1 = t2;
        }

        t2 = glfwGetTime();
    }
    // -------------------------------------------------------

	terminate(window);
}