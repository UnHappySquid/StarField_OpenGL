#include "helper.h"

using namespace std;

const int num_stars = 2500;
const float init_star_size = 0.001;
Position init_speed = { 0, 0, 0, 1 };
const float max_speed_z = 5;
const float max_speed_xy = 5;
const float min_speed = 0.01;
const float slow_down_rate = 0.90;
const int num_points_per_circle = 10;

// Generates the stars and holds main way to make movement
class Starfield {

    // Circle holds the OpenGL implementation details to get a cirle on the screen
    // Star holds that and the implementation to get a line behind the star
    class Star : protected Circle<num_points_per_circle> {

    public:
        // No color specified (white)
        Star(Position center) : Circle{ center, init_star_size, 1, 1, 1 } {
        }
        // Complete constructor
        Star(Position center, float r, float g, float b) : Circle{ center, init_star_size, r, g, b } {
        }

        void bound_check_logic() {
            Position p = get_center();
            Position new_center = p;

            if (p.x <= -2.5f || p.x >= 2.5f) {
                new_center.x = p.x <= -2.5f ? generate_xy_far() : -generate_xy_far();
            }
            if (p.y <= -2.5f || p.y >= 2.5f) {
                new_center.y = p.y <= -2.5f ? generate_xy_far() : -generate_xy_far();

            }
            if (p.z < -5.f || p.z > 5.f) {
                new_center.z = p.z < -5.f ? generate_z_far() : -generate_z_far();
            }
            if (p != new_center) {
                move_all_to(new_center);
            }
        }

        void tick(float dt, Position velocity) {
            bound_check_logic();
            move_all_by(velocity.x * dt, velocity.y * dt, velocity.z * dt);
            Circle::draw();
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
            new_c.z = -curr_c.x * sin(to_rad(deg)) + curr_c.z * cos(to_rad(deg));
            new_c.y = curr_c.y;
            new_c.w = curr_c.w;
            move_all_to(new_c);
        }

        void resize(float dr) {
            Circle::resize(dr);
        }
    };

    vector<Star*> stars;
    Position field_velocity;
    bool slowing_down = false;
public:
    Starfield(int num_stars, Position vel) : field_velocity{ vel } {

        for (size_t i = 0; i < num_stars; i++)
        {
            // Generate random position in x, y, z
            Position ran_pos{ generate_xy(),
            generate_xy(),
            generate_z(),
            1 };

            // Make new star with pastel colors
            Star* star = new Star(ran_pos, 0.8 + generate_color() / 5
                , 0.8 + generate_color() / 5, 0.8 + generate_color() / 5);
            // Push back into star vector 
            stars.push_back(star);
        }
    }

    void tick(float dt) {
        if (slowing_down && get_speed() >= min_speed) {
            field_velocity.x *= slow_down_rate;
            field_velocity.y *= slow_down_rate;
            field_velocity.z *= slow_down_rate;
        }
        else if (slowing_down && get_speed() < min_speed) {
            field_velocity.x = 0;
            field_velocity.y = 0;
            field_velocity.z = 0;
            slowing_down = false;
        }
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

    void resize_all(float dr) {
        for (Star* s : stars) {
            s->resize(dr);
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

    float get_speed() {
        return sqrt(field_velocity.x * field_velocity.x + field_velocity.y * field_velocity.y + field_velocity.z * field_velocity.z);
    }

    void set_slowing_down(bool b) {
        slowing_down = b;
    }

    bool get_slowing_down() {
        return slowing_down;
    }

};

void process_input(GLFWwindow* window, Starfield& field) {
    // if we are slowing down then dont process inputs for speed
    if (!field.get_slowing_down()) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
            field.get_velocity().z >= -max_speed_z) {
            field.get_velocity_ref().z -= 0.01;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS
            && field.get_velocity().z <= max_speed_z) {
            field.get_velocity_ref().z += 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS
            && field.get_velocity().x <= max_speed_xy) {
            field.get_velocity_ref().x += 0.01;

        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS
            && field.get_velocity().x >= -max_speed_xy) {
            field.get_velocity_ref().x -= 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS
            && field.get_velocity().y <= max_speed_xy) {
            field.get_velocity_ref().y += 0.01;

        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
            && field.get_velocity().y >= -max_speed_xy) {
            field.get_velocity_ref().y -= 0.01;
        }
    }
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        field.set_slowing_down(true);
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

    // Not sure if i wanna keep this resize part
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {

        field.resize_all(0.0005);
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {

        field.resize_all(-0.0005);
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
	unsigned int prog = create_and_use_shaders(vs, fs);



    unsigned int frustumScaleUnif = glGetUniformLocation(prog, "frustumScale");
    unsigned int zNearUnif = glGetUniformLocation(prog, "zNear");
    unsigned int zFarUnif = glGetUniformLocation(prog, "zFar");
    unsigned int aspectUnif = glGetUniformLocation(prog, "aspect");

    glUniform1f(frustumScaleUnif, f);
    glUniform1f(zNearUnif, Znear);
    glUniform1f(zFarUnif, Zfar);
    glUniform1f(aspectUnif, aspect);

	// Set up here --------------------------------------------
	double t1 = 0;
	double t2 = glfwGetTime();
    //  Background color
	glClearColor(0.1, 0.1, 0.1, 1);
	// StarField
    Starfield field{num_stars, init_speed};
    // -------------------------------------------------------
    
    // Game Loop ---------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        if (t2 - t1 >= frame_duration) {
            // Resize the 
            FOV = field.get_speed() / sqrt(2 * (max_speed_xy * max_speed_xy) + max_speed_z * max_speed_z) * 360 + 45;
            f = 1. / tanf((FOV * PI / 180) / 2);
            glUniform1f(frustumScaleUnif, f);

            glClear(GL_COLOR_BUFFER_BIT);
            field.tick(frame_duration);
            glFinish();
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