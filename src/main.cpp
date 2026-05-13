#include <cstdio>
#include <cmath>
#include <vector>
#include <random>
#include <GLFW/glfw3.h>

struct Vec2 { 
    float x, y; 
};

struct Particle {
    Vec2 pos, vel, acc; 
    float mass, r, ela; // mass, radius, elasticity
};

// Math helpers for vector calculations
inline Vec2 operator+(Vec2 a, Vec2 b) {
    return { a.x + b.x, a.y + b.y };
}

inline Vec2 operator*(float c, Vec2 a) {
    return { a.x * c, a.y * c };
}

inline Vec2 operator/(Vec2 v, float s) {
    if (std::fabs(s) < 1e-8f) return {0,0};
    return { v.x / s, v.y / s };
}

inline float dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

// Drawing a simple OpenGL circle
static void draw_circle_filled(float cx, float cy, float r, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i) {
        float a = (2.0f * 3.1415926f) * (float)i / (float)segments;
        float x = cx + std::cos(a) * r;
        float y = cy + std::sin(a) * r;
        glVertex2f(x, y);
    }
    glEnd();
}

// Elastic collision
static void collide(Particle& p, Particle& sp) {
    Vec2 rp { p.pos.x - sp.pos.x, p.pos.y - sp.pos.y };
    float dist = std::sqrt(dot(rp, rp));
    float rsum  = p.r + sp.r;
    Vec2 normal = (dist > 0) ? rp/dist: Vec2 {0.0f,1.0f};

    float pen = rsum - dist; // overlap

    float p_mass_frac = p.mass/(p.mass+sp.mass);
    float sp_mass_frac = sp.mass/(p.mass+sp.mass);

    // Move the particles apart
    p.pos  = p.pos + pen*sp_mass_frac*normal;
    sp.pos = sp.pos + -1*pen*p_mass_frac*normal;

    Vec2 rv { p.vel.x - sp.vel.x, p.vel.y - sp.vel.y };
    float vn = dot(rv, normal);

    float res = std::min(p.ela, sp.ela);

    // swap velocities
    if (vn < 0.0f) {
        p.vel  = p.vel + -1*(1+res)*vn*sp_mass_frac*normal;
        sp.vel = sp.vel + (1+res)*vn*p_mass_frac*normal;
    }
}

int main() {
    if (!glfwInit()) { std::printf("glfwInit failed\n"); return 1; }

    const int height = 900;
    const int width = 900;

    // Create window
    GLFWwindow* win = glfwCreateWindow(width, height, "Ball Pit", nullptr, nullptr);
    if (!win) { std::printf("glfwCreateWindow failed\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);
    
    // -------------- SETUP ----------------
    std::mt19937 rng(42);
    
    std::vector<Particle> ps;
    std::uniform_real_distribution<float> random_pos(-0.9f, 0.9f);
    std::uniform_real_distribution<float> random_vel(-0.5f, 0.5f);
    std::uniform_real_distribution<float> random_mass(10.0f, 12.0f);

    const int np = 150; // total number of balls

    ps.reserve(np);

    // stats for balls
    for (int i = 0; i < np; ++i) {
        Particle p;
        p.pos = { random_pos(rng), random_pos(rng) };
        p.vel = { random_vel(rng), random_vel(rng) };
        p.acc = { 0.0f, 0.0f };
        p.mass = random_mass(rng);
        p.r = std::sqrt(p.mass)/100;
        p.ela = 0.6f;
        ps.push_back(p);
    }

    const float dt = 1.0f/240.0f; // timestep
    const float friction = 0.1f; 
    float f = 1.0f - friction * dt;

    float acc_x = 0.0f;
    float acc_y = -9.81f; // earth gravity

    const float push_force = 150.0f;
    
    const float min_x = -1.0f, max_x = 1.0f;
    const float min_y = -1.0f, max_y = 1.0f;

    // Fake particle used for wall collisions
    Particle wall;
    wall.vel = { 0.0f, 0.0f };
    wall.acc = { 0.0f, 0.0f };
    wall.mass = 10000000.0f;
    wall.r = 0.05f;
    wall.ela = 0.6f;

    // --------------------------------------

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();    
        
        for (auto& p : ps) {

            p.acc.x = acc_x;
            p.acc.y = acc_y;
            
            // Press space to turn off gravity
            if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) {p.acc.x = 0; p.acc.y = 0;}

            // Use WASD to move all balls
            if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) p.acc.x -= push_force / p.mass;
            if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) p.acc.x += push_force / p.mass;
            if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) p.acc.y -= push_force / p.mass;
            if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) p.acc.y += push_force / p.mass;
            
            // Update velocities
            p.vel.x += p.acc.x * dt;
            p.vel.y += p.acc.y * dt;

            // Update positions
            p.pos.x += p.vel.x * dt;
            p.pos.y += p.vel.y * dt;

        }

        // Substeps for better stability
        for (int k = 0; k < 32; k++) {
            for (int i = 0; i < np; ++i) {
                for (int j = i + 1; j < np; ++j) {
                    float rsum = ps[i].r + ps[j].r; 
                    float dx = ps[i].pos.x - ps[j].pos.x;
                    // Distance check to save processing power
                    if (dx < rsum) {
                        float dy = ps[i].pos.y - ps[j].pos.y;
                        // Another distance check
                        if (dy < rsum) {
                            Vec2 rp { dx, dy }; 
                            // Last distance check
                            float dist = std::sqrt(dot(rp, rp)); 
                            if (dist < rsum) { 
                                // If balls are truly close enough, collide them
                                collide(ps[i], ps[j]);
                            }
                        }
                    }
                }
            }

            // Check for collisions with the wall
            for (auto& p : ps) {

                if (p.pos.x+p.r > max_x-wall.r) { wall.pos = { max_x, p.pos.y }; collide(p, wall); }
                if (p.pos.x-p.r < min_x+wall.r) { wall.pos = { min_x, p.pos.y }; collide(p, wall); }
                if (p.pos.y+p.r > max_y-wall.r) { wall.pos = { p.pos.x, max_y }; collide(p, wall); }
                if (p.pos.y-p.r < min_y+wall.r) { wall.pos = { p.pos.x, min_y }; collide(p, wall); }
            }
        }

        // Air resistance
        for (auto& p : ps) {
            p.vel = f * p.vel;
        }

        // Render
        glViewport(0, 0, width, height); 
        glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        for (const auto& p : ps) {
            draw_circle_filled(p.pos.x, p.pos.y, p.r, (int)p.mass+10);
        }
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
