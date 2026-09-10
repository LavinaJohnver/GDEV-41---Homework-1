#include "raylib.h"
#include "cmath"

#include "fstream"
#include "sstream"
#include "iostream"
#include "string"

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

void load_settings(const std::string &filepath, bool &hideUI,
    int &outIncreaseRateX, int &outDecreaseRateX, int &outEmitKeyX, 
    int &outIncreaseRateY, int &outDecreaseRateY, int &outEmitButtonY) {
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << "!" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') { // skip empty lines + comments
            continue;
        }

        std::istringstream line_stream(line);
        std::string key, value_str;

        if (std::getline(line_stream, key, '=') && std::getline(line_stream, value_str)) {
            if (key == "hideUI") {
                hideUI = (value_str == "true" || value_str == "1");
                continue;
            }

            try {
                int value = std::stoi(value_str);
                if (key == "IncreaseRateX") outIncreaseRateX = value;
                else if (key == "DecreaseRateX") outDecreaseRateX = value;
                else if (key == "EmitKeyX") outEmitKeyX = value;
                else if (key == "IncreaseRateY") outIncreaseRateY = value;
                else if (key == "DecreaseRateY") outDecreaseRateY = value;
                else if (key == "EmitButtonY") outEmitButtonY = value;
            } catch (const std::invalid_argument &e) {
                std::cerr << "Error: Invalid value for " << key << ": " << value_str << " in " << filepath << "\n";
            }
        }
    }
    file.close();
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
    const int FPS = 60;

    InitWindow(screenWidth, screenHeight, "Particle System");
    SetTargetFPS(FPS);

    // get keybinds from config.ini
    int keyIncreaseRateX = KEY_RIGHT;
    int keyDecreaseRateX = KEY_LEFT;
    int keyEmitX         = KEY_SPACE;
    int keyIncreaseRateY = KEY_UP;
    int keyDecreaseRateY = KEY_DOWN;
    int mouseEmitButtonY = MOUSE_BUTTON_LEFT;

    bool hideUI = true;

    load_settings("config.ini", hideUI,
        keyIncreaseRateX, keyDecreaseRateX, keyEmitX,
        keyIncreaseRateY, keyDecreaseRateY, mouseEmitButtonY);

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
        if (IsKeyDown(keyDecreaseRateX)) rateX -= 20.0f * dt;
        if (IsKeyDown(keyIncreaseRateX)) rateX += 20.0f * dt;
        if (IsKeyDown(keyIncreaseRateY)) rateY += 20.0f * dt;
        if (IsKeyDown(keyDecreaseRateY)) rateY -= 20.0f * dt;
        
        // threshold for min max spawn rate
        if (rateX < 1.0f) rateX = 1.0f;
        if (rateX > 50.0f) rateX = 50.0f;
        if (rateY < 1.0f) rateY = 1.0f;
        if (rateY > 50.0f) rateY = 50.0f;
        
        // SPACEBAR to spawn particles
        if (IsKeyDown(keyEmitX))
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

        if (IsMouseButtonDown(mouseEmitButtonY))
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

        if (hideUI == false){
            DrawText(TextFormat("Spacebar Rate (X): %.1f/sec (Left/Right to change)",rateX),10,10,20,RAYWHITE);
            DrawText(TextFormat("Mouse Rate (Y): %.1f/sec (Up/Down to change)",rateY),10,30,20,RAYWHITE);
        }
        
        EndDrawing();
    }
    delete[] particles;

    CloseWindow();

    return 0;
}