#include "raylib.h"

struct Particle
{
    bool isActive;
    Vector2 position;
    Vector2 direction;
    float speed;
    float lifeTime;
    Color color;
};

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Particle System");
    SetTargetFPS(60);

    const int particleCount = 1000;
    Particle *particles = new Particle[particleCount];

    for (int i = 0; i < particleCount; i++)
    {
        particles[i].isActive = false;
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    delete[] particles;

    CloseWindow();

    return 0;
}
