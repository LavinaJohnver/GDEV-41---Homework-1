#include <raylib.h>
#include "raymath.h"
#include <vector>
#include <cmath>

const int   WINDOW_WIDTH  = 1280;
const int   WINDOW_HEIGHT = 720;
const float FPS           = 60.0f;
const float TIMESTEP      = 1.0f / FPS;

const float MIN_RADIUS = 5.0f;
const float MAX_RADIUS = 10.0f;

const float ELASTICITY      = 0.92f; 
const float WALL_ELASTICITY = 0.88f; 
const float CELL_SIZE = MAX_RADIUS * 2.0f;
const float SPAWN_SPEED_MIN = 100.0f;
const float SPAWN_SPEED_MAX = 300.0f;

struct Ball {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float inverse_mass;   // 1 / mass, since every impulse divides by mass
    Color color;
};

struct GridCell {
    Vector2 position;
    float width;
    float height;
};

static Ball MakeBall(const Vector2& at) {
    float radius = (float)GetRandomValue((int)MIN_RADIUS, (int)MAX_RADIUS);

    float mass = radius * radius;

    // random direction + speed
    float angle = (float)GetRandomValue(0, 359) * DEG2RAD;
    float speed = (float)GetRandomValue((int)SPAWN_SPEED_MIN, (int)SPAWN_SPEED_MAX);

    Color color = ColorFromHSV((float)GetRandomValue(0, 359), 0.65f, 0.95f);

    return Ball{
        at, { cosf(angle) * speed, sinf(angle) * speed }, radius, 1.0f / mass, color
    };
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Grid-Based Ball Collision");
    SetTargetFPS((int)FPS);

    const Vector2 spawnPoint = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };

    std::vector<Ball> balls;

    float accumulator = 0.0f;
    float spawnTimer = 0.0f;
    bool  showGrid    = false;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)){
            balls.push_back(MakeBall(spawnPoint));
            spawnTimer = 0.0f;
        }
        
        if (IsKeyPressed(KEY_G))
            showGrid  = !showGrid; 

        // Physics step
        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {
            // ------ SEMI-IMPLICIT EULER INTEGRATION -------
            // Sequential motion: each ball fully computes its velocity, position, and
            // collisions against every other ball before moving on to the next ball
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (showGrid) {
            for (int c = 1; c < (WINDOW_WIDTH + (int)CELL_SIZE - 1) / (int)CELL_SIZE; ++c)
            {
                DrawLine(c * (int)CELL_SIZE, 0, c * (int)CELL_SIZE, WINDOW_HEIGHT, GRAY);
            }
        }

        for (const auto& b : balls) {
            DrawCircleV(b.position, b.radius, b.color);
            DrawCircleLinesV(b.position, b.radius, Fade(BLACK, 0.35f));
        }

        int naive = (int)balls.size() * ((int)balls.size() - 1) / 2;
        //DrawText(TextFormat("PARTICLES %d", GetParticles()), 12, 60, 20, RAYWHITE);
        DrawText("SPACE spawn   Q to toggle grid",
                 12, WINDOW_HEIGHT - 28, 18, Fade(RAYWHITE, 0.6f));

        EndDrawing();
    }
}