#include <raylib.h>
#include <raymath.h>
#include <vector>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
const float FRICTION = 0.5;
float elasticity = 1.0f; // Toggled between 0 (sticky) and 1 (bouncy) by pressing SPACE
const Color BACKGROUND_COLOR = { 67, 173, 101, 255 }; 

struct Ball {
    Vector2 position;
    float radius;
    Color color;

    float mass;
    float inverse_mass; // A variable for 1 / mass. Used in the calculation for acceleration = sum of forces / mass
    Vector2 acceleration;
    Vector2 velocity;
};

void CollisionTypeShi(Ball& b1, Ball& b2) {

    Vector2 colNormal = Vector2Subtract(b2.position, b1.position);
    float dist = Vector2Length(colNormal);

    float minDist = b1.radius + b2.radius;
    if (dist >= minDist) return;

    if (dist == 0.0f) {
        dist = 0.1f;
        colNormal = { 1.0f, 0.0f };
    } else {
        colNormal = Vector2Normalize(colNormal);
    }

    Vector2 relativeVelocity = Vector2Subtract(b2.velocity, b1.velocity);
    float velNormal = Vector2DotProduct(relativeVelocity, colNormal);

    float impulse = (-(1 + elasticity) * velNormal) / (Vector2DotProduct(colNormal, colNormal) * (b1.inverse_mass + b2.inverse_mass));
    b2.velocity = Vector2Add(b2.velocity, Vector2Scale(colNormal, impulse * b2.inverse_mass));
    b1.velocity = Vector2Subtract(b1.velocity, Vector2Scale(colNormal, impulse * b1.inverse_mass));
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Demo");

    SetTargetFPS(FPS);

    float accumulator = 0;

    Vector2 mouseDragStart = Vector2Zero();
    bool isDragging = false;
    const float MAX_DRAG_DISTANCE = 150.0f; 
    const float FORCE_MULTIPLIER = 8.0f;    
    std::vector<Ball> balls = {
        { {100, 300}, 40.0f, WHITE,  1.0f, 1.0f, Vector2Zero(), {0, 0} },
        { {400, 300}, 40.0f, BLUE, 1.0f, 1.0f, Vector2Zero(), {0, 0} },
        { {470, 240}, 40.0f, BLUE, 1.0f, 1.0f, Vector2Zero(), {0, 0} },
        { {470, 360}, 40.0f, BLUE, 1.0f, 1.0f, Vector2Zero(), {0, 0} },
        { {540, 300}, 40.0f, BLUE, 1.0f, 1.0f, Vector2Zero(), {0, 0} }
    };


    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        // Pressing SPACE toggles elasticity between 0 (objects stick more) and 1 (objects bounce more)
        if (IsKeyPressed(KEY_SPACE)) {
            elasticity = (elasticity == 0.0f) ? 1.0f : 0.0f;
        }
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            mouseDragStart = GetMousePosition();
            isDragging = true;
        }

        if (isDragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            Vector2 dragVector = Vector2Subtract(mouseDragStart, mousePos);
            float dragDistance = fminf(Vector2Length(dragVector), MAX_DRAG_DISTANCE);

            if (dragDistance > 0.0f) {
                Vector2 forceDir = Vector2Normalize(dragVector);
                Vector2 impulse = Vector2Scale(forceDir, dragDistance * FORCE_MULTIPLIER);
                balls[0].velocity = Vector2Add(balls[0].velocity, Vector2Scale(impulse, balls[0].inverse_mass));
            }

            isDragging = false;
        }

        // Physics step
        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {
            // ------ SEMI-IMPLICIT EULER INTEGRATION -------
            // Sequential motion: each ball fully computes its velocity, position, and
            // collisions against every other ball before moving on to the next ball
            for (int i = 0; i < balls.size(); ++i) {
                Ball& b1 = balls[i];

                // Computes for velocity using v(t + dt) = v(t) + (a(t) * dt), then applies friction
                b1.velocity = Vector2Add(b1.velocity, Vector2Scale(b1.acceleration, TIMESTEP));
                b1.velocity = Vector2Subtract(b1.velocity, Vector2Scale(b1.velocity, FRICTION * b1.inverse_mass * TIMESTEP));

                b1.position = Vector2Add(b1.position, Vector2Scale(b1.velocity, TIMESTEP));

                for (int j = 0; j < balls.size(); ++j) {
                    if (j == i) continue;
                    CollisionTypeShi(b1, balls[j]);
                }

                // Negates the velocity at x and y if the object hits a wall. (Basic Collision Detection)
                if(b1.position.x + b1.radius >= WINDOW_WIDTH || b1.position.x - b1.radius <= 0) {
                    b1.velocity.x *= -1;
                }
                if(b1.position.y + b1.radius >= WINDOW_HEIGHT || b1.position.y - b1.radius <= 0) {
                    b1.velocity.y *= -1;
                }
            }

            accumulator -= TIMESTEP;
        }
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        for (const auto& b : balls) {
            DrawCircleV(b.position, b.radius, b.color);
        }
        if (isDragging) {
            DrawLineV(mouseDragStart, GetMousePosition(), DARKGRAY);
            DrawCircleV(mouseDragStart, 5.0f, DARKGRAY);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

