#include "raylib.h"
#include "cmath"

struct Particle
{
    bool isActive;
    Vector2 position;
    Vector2 direction;
    float speed;
    float lifeTime;
    float maxLifeTime;
    Color color;
};

// for generating random floats between min and max
float GetRandomFloat(float min, float max)
{
    float scale = GetRandomValue(0,1000)/1000.0f;
    return min + scale * (max-min);
};

// for generating random colors
Color GetRandomColor()
{
    return Color{
        (unsigned char)GetRandomValue(0,255),
        (unsigned char)GetRandomValue(0,255),
        (unsigned char)GetRandomValue(0,255),
        255
    };
}

Vector2 NormalizeDirection(Vector2 dir)
{
    float length = sqrtf(dir.x*dir.x + dir.y*dir.y);
    if (length > 0.0f)
    {
        dir.x /= length;
        dir.y /= length;
    }
    return dir;
}
void EmitParticle(Particle *particles, int particleCount, Vector2 pos, Vector2 dir, float speed, float lifetime, Color color)
{
    for(int i = 0; i <particleCount; i++)
    {
        if(!particles[i].isActive)
        {
            particles[i].isActive = true;
            particles[i].position = pos;
            particles[i].direction = NormalizeDirection(dir);
            particles[i].speed = speed;
            particles[i].lifeTime = lifetime;
            particles[i].maxLifeTime = lifetime;
            particles[i].color = color;
            break;
        }
    }
}

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

    float rateX = 20.0f; //default emission rate
    float timeX = 0.0f; //controls emission intervals

    float rateY = 20.0f;
    float timeY = 0.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        
        // adjusts spawn rate
        if (IsKeyDown(KEY_LEFT)) rateX -= 20.0f * dt;
        if (IsKeyDown(KEY_RIGHT)) rateX += 20.0f * dt;
        if (IsKeyDown(KEY_UP)) rateY += 20.0f * dt;
        if (IsKeyDown(KEY_DOWN)) rateY -= 20.0f * dt;
        
        // threshold for min max spawn rate
        if (rateX < 1.0f) rateX = 1.0f;
        if (rateX > 50.0f) rateX = 50.0f;
        if (rateY < 1.0f) rateY = 1.0f;
        if (rateY > 50.0f) rateY = 50.0f;
        
        // SPACEBAR to spawn particles
        if (IsKeyDown(KEY_SPACE))
        {
            timeX += dt;
            float intervalX = 1.0f/rateX;
            while (timeX >= intervalX) // emmits after enough time elapsed
            {
                Vector2 spawnPos = {(float)screenWidth/2.0f,(float)screenHeight};
                Vector2 dir = {GetRandomFloat(-1.0f,1.0), -1.0f};
                
                float speed = GetRandomFloat(50.0f,100.0f);
                float lifetime = GetRandomFloat(2.0f,5.0f);
                Color color = GetRandomColor();
                
                EmitParticle(particles, particleCount, spawnPos, dir, speed, lifetime, color);
                
                timeX -= intervalX;
            }
        }
        else timeX = 0.0f;

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            timeY += dt;
            float intervalY = 1.0f/rateY;
            while (timeY >= intervalY)
            {
                Vector2 spawnPos = GetMousePosition();
 
                float angle = GetRandomFloat(0.0f, 2.0f*PI); // random spread
                Vector2 dir = {cosf(angle), sinf(angle)};
 
                float speed = GetRandomFloat(50.0f,100.0f);
                float lifetime = GetRandomFloat(0.5f,2.0f);
                Color color = GetRandomColor();
 
                EmitParticle(particles, particleCount, spawnPos, dir, speed, lifetime, color);
 
                timeY -= intervalY;
            }
        }
        else timeY = 0.0f;
        
        for (int i = 0; i < particleCount; i++)
        {
            if(particles[i].isActive)
            {
                // update position and speed
                particles[i].position.x += particles[i].direction.x * particles[i].speed * dt;
                particles[i].position.y += particles[i].direction.y * particles[i].speed * dt;
                
                // updates lifetime
                particles[i].lifeTime -= dt;
                if (particles[i].lifeTime <= 0.0f)
                {
                    particles[i].isActive = false;
                }
            }
        }
        
        BeginDrawing();
        ClearBackground(BLACK);
        
        for(int i = 0; i < particleCount; i++)
        {
            if (particles[i].isActive)
            {
                // fade opacity from 100% down to 0% as lifeTime runs out
                float lifeFraction = particles[i].lifeTime / particles[i].maxLifeTime;
                if (lifeFraction < 0.0f) lifeFraction = 0.0f;
                if (lifeFraction > 1.0f) lifeFraction = 1.0f;

                Color drawColor = particles[i].color;
                drawColor.a = (unsigned char)(lifeFraction * 255.0f);

                DrawCircleV(particles[i].position, 5.0f, drawColor);
            }
        }
        DrawText(TextFormat("Spacebar Rate (X): %.1f/sec (Left/Right to change)",rateX),10,10,20,RAYWHITE);
        DrawText(TextFormat("Mouse Rate (Y): %.1f/sec (Up/Down to change)",rateY),10,30,20,RAYWHITE);
        
        EndDrawing();
    }
    delete[] particles;

    CloseWindow();

    return 0;
}
