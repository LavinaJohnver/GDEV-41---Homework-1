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
const float BORDER_THICKNESS = 30.0f;
const float POCKET_RADIUS = 30.0f;
const Color TABLE_BORDER_COLOR = { 101, 67, 33, 255 };
const Color POCKET_COLOR = BLACK;

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
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pool");

    SetTargetFPS(FPS);

    float accumulator = 0;

    Vector2 mouseDragStart = Vector2Zero();
    bool isDragging = false;
    const float MAX_DRAG_DISTANCE = 150.0f;
    const float FORCE_MULTIPLIER = 8.0f;
    std::vector<Ball> balls = {
        { {100, 300}, 20.0f, WHITE,  1.0f, 1.0f, Vector2Zero(), {0, 0} },
        { {400, 300}, 20.0f, BLUE, 1.0f, 1.0f, Vector2Zero(), {0, 0} },
        { {470, 240}, 20.0f, BLUE, 1.0f, 1.0f, Vector2Zero(), {0, 0} },
        { {470, 360}, 20.0f, BLUE, 1.0f, 1.0f, Vector2Zero(), {0, 0} },
        { {540, 300}, 20.0f, BLUE, 1.0f, 1.0f, Vector2Zero(), {0, 0} }
    };

    Vector2 pockets[6] = {                                                
            { BORDER_THICKNESS, BORDER_THICKNESS },
            { WINDOW_WIDTH / 2.0f, BORDER_THICKNESS },
            { WINDOW_WIDTH - BORDER_THICKNESS, BORDER_THICKNESS },
            { BORDER_THICKNESS, WINDOW_HEIGHT - BORDER_THICKNESS },
            { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - BORDER_THICKNESS },
            { WINDOW_WIDTH - BORDER_THICKNESS, WINDOW_HEIGHT - BORDER_THICKNESS }
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

                if (b1.position.x - b1.radius <= BORDER_THICKNESS) {
                    b1.position.x = BORDER_THICKNESS + b1.radius;
                    b1.velocity.x *= -1;
                } else if (b1.position.x + b1.radius >= WINDOW_WIDTH - BORDER_THICKNESS) {
                    b1.position.x = WINDOW_WIDTH - BORDER_THICKNESS - b1.radius;
                    b1.velocity.x *= -1;
                }

                if (b1.position.y - b1.radius <= BORDER_THICKNESS) {
                    b1.position.y = BORDER_THICKNESS + b1.radius;
                    b1.velocity.y *= -1;
                } else if (b1.position.y + b1.radius >= WINDOW_HEIGHT - BORDER_THICKNESS) {
                    b1.position.y = WINDOW_HEIGHT - BORDER_THICKNESS - b1.radius;
                    b1.velocity.y *= -1;
                }
                
                for (const auto& pocket : pockets) {
                    if (CheckCollisionCircles(b1.position, b1.radius * 0.5f, pocket, POCKET_RADIUS)) {
                        if (i == 0) {
                            b1.position = { 150, 300 };      // Reset Cue Ball
                            b1.velocity = Vector2Zero();
                        } else {
                            b1.position = { -1000, -1000 };  // Hide Ball
                            b1.velocity = Vector2Zero();
                        }
                        break;
                    }
                }
            }

            accumulator -= TIMESTEP;
        }
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        
        // 6 Pool Pockets
        for (const auto& pocket : pockets) {
            DrawCircleV(pocket, POCKET_RADIUS, POCKET_COLOR);
        }

        // Pool Border
        DrawRectangle(0, 0, WINDOW_WIDTH, BORDER_THICKNESS, TABLE_BORDER_COLOR);
        DrawRectangle(0, WINDOW_HEIGHT - BORDER_THICKNESS, WINDOW_WIDTH, BORDER_THICKNESS, TABLE_BORDER_COLOR);
        DrawRectangle(0, 0, BORDER_THICKNESS, WINDOW_HEIGHT, TABLE_BORDER_COLOR);
        DrawRectangle(WINDOW_WIDTH - BORDER_THICKNESS, 0, BORDER_THICKNESS, WINDOW_HEIGHT, TABLE_BORDER_COLOR);
        
        // Pool Balls
        for (const auto& b : balls) {
            if (b.position.x > 0) {
                DrawCircleV(b.position, b.radius, b.color);
            }
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

