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
const float BIG_RADIUS = 25.0f;

const float CELL_SIZE = BIG_RADIUS * 2.0f;
const float SPAWN_SPEED_MIN = 100.0f;
const float SPAWN_SPEED_MAX = 300.0f;

const int GRID_COLS = (WINDOW_WIDTH  + (int)CELL_SIZE - 1) / (int)CELL_SIZE;
const int GRID_ROWS = (WINDOW_HEIGHT + (int)CELL_SIZE - 1) / (int)CELL_SIZE;

int SPAWN_COUNT = 0;
int PARTICLE_COUNT = 0;

struct Ball {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float inverse_mass;
    Color color;
};

struct GridCell {
    Vector2 position;
    float width;
    float height;

    // stores balls that are colliding within the cell
    std::vector<int> ball_indices;
};

static Ball SpawnBall(const Vector2& at, bool big) {
    float radius = (float)GetRandomValue((int)MIN_RADIUS, (int)MAX_RADIUS);
    
    if (big == true) radius = BIG_RADIUS;

    PARTICLE_COUNT++;

    float mass = radius * radius;

    // random direction + speed
    float angle = (float)GetRandomValue(0, 359) * DEG2RAD;
    float speed = (float)GetRandomValue((int)SPAWN_SPEED_MIN, (int)SPAWN_SPEED_MAX);

    Color color = ColorFromHSV((float)GetRandomValue(0, 359), 0.65f, 0.95f);

    return Ball{
        at, { cosf(angle) * speed, sinf(angle) * speed }, radius, 1.0f / mass, color
    };
}

static std::vector<GridCell> CreateGrid() {
    std::vector<GridCell> grid;
    grid.reserve(GRID_COLS * GRID_ROWS);

    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            Vector2 position = { col * CELL_SIZE, row * CELL_SIZE };

            float width  = fminf(CELL_SIZE, WINDOW_WIDTH  - position.x);
            float height = fminf(CELL_SIZE, WINDOW_HEIGHT - position.y);

            grid.push_back(GridCell{ position, width, height, {} });
        }
    }

    return grid;
}

static void BuildGrid(const std::vector<Ball>& balls, std::vector<GridCell>& grid) {
    for (GridCell& cell : grid)
        cell.ball_indices.clear();

    for (int i = 0; i < (int)balls.size(); ++i) {
        const Ball& b = balls[i];

        // Determine bounding box in grid coordinates to handle multi-cell overlaps
        int min_col = (int)Clamp((b.position.x - b.radius) / CELL_SIZE, 0.0f, GRID_COLS - 1.0f);
        int max_col = (int)Clamp((b.position.x + b.radius) / CELL_SIZE, 0.0f, GRID_COLS - 1.0f);
        int min_row = (int)Clamp((b.position.y - b.radius) / CELL_SIZE, 0.0f, GRID_ROWS - 1.0f);
        int max_row = (int)Clamp((b.position.y + b.radius) / CELL_SIZE, 0.0f, GRID_ROWS - 1.0f);

        for (int r = min_row; r <= max_row; ++r) {
            for (int c = min_col; c <= max_col; ++c) {
                grid[r * GRID_COLS + c].ball_indices.push_back(i);
            }
        }
    }
}
static void ResolveCollision(Ball& a, Ball& b) {
    Vector2 delta    = Vector2Subtract(b.position, a.position);
    float   distance = Vector2Length(delta);
    float   contact  = a.radius + b.radius;

    if (distance >= contact) return;

    Vector2 normal;
    if (distance > 0.0001f) {
        normal = Vector2Scale(delta, 1.0f / distance);
    } else {
        normal   = { 1.0f, 0.0f };
        distance = 0.0f;
    }

    float inverse_mass_sum = a.inverse_mass + b.inverse_mass;
    if (inverse_mass_sum <= 0.0f) return;

    float   penetration = contact - distance;
    Vector2 correction  = Vector2Scale(normal, penetration / inverse_mass_sum);

    a.position = Vector2Subtract(a.position, Vector2Scale(correction, a.inverse_mass));
    b.position = Vector2Add(     b.position, Vector2Scale(correction, b.inverse_mass));

    // --- impulse ---
    Vector2 relative_velocity = Vector2Subtract(b.velocity, a.velocity);
    float   approach_speed    = Vector2DotProduct(relative_velocity, normal);

    if (approach_speed > 0.0f) return;

    float   impulse        = -2.0f * approach_speed / inverse_mass_sum;
    Vector2 impulse_vector = Vector2Scale(normal, impulse);

    a.velocity = Vector2Subtract(a.velocity, Vector2Scale(impulse_vector, a.inverse_mass));
    b.velocity = Vector2Add(     b.velocity, Vector2Scale(impulse_vector, b.inverse_mass));
}

static void ResolveCollisions(std::vector<Ball>& balls, const std::vector<GridCell>& grid) {
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {

            const GridCell& cell = grid[row * GRID_COLS + col];
            if (cell.ball_indices.empty()) continue;

            for (int neighbour_row = row - 1; neighbour_row <= row + 1; ++neighbour_row) {
                if (neighbour_row < 0 || neighbour_row >= GRID_ROWS) continue;

                for (int neighbour_col = col - 1; neighbour_col <= col + 1; ++neighbour_col) {
                    if (neighbour_col < 0 || neighbour_col >= GRID_COLS) continue;

                    const GridCell& neighbour = grid[neighbour_row * GRID_COLS + neighbour_col];

                    for (int i : cell.ball_indices) {
                        for (int j : neighbour.ball_indices) {
                            if (j <= i) continue;
                            ResolveCollision(balls[i], balls[j]);
                        }
                    }
                }
            }
        }
    }
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Grid-Based Ball Collision");
    SetTargetFPS((int)FPS);

    const Vector2 spawnPoint = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };

    std::vector<Ball> balls;

    std::vector<GridCell> grid = CreateGrid();

    float accumulator = 0.0f;
    bool  showGrid    = true ;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)){
            if((SPAWN_COUNT+1) % 11 == 0 && SPAWN_COUNT != 0){
                balls.push_back(SpawnBall(spawnPoint, true));
                SPAWN_COUNT++;
            } else{
                for(int i = 0; i < 25; i++){
                    balls.push_back(SpawnBall(spawnPoint, false));
                }
                SPAWN_COUNT++;
            }
        }
        
        if (IsKeyPressed(KEY_Q))
            showGrid  = !showGrid;

        // Physics step
        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {
            for (Ball& ball : balls) {
                ball.position = Vector2Add(ball.position, Vector2Scale(ball.velocity, TIMESTEP));

                if (ball.position.x - ball.radius <= 0.0f) {
                    ball.position.x = ball.radius;
                    ball.velocity.x = -ball.velocity.x;
                } else if (ball.position.x + ball.radius >= WINDOW_WIDTH) {
                    ball.position.x = WINDOW_WIDTH - ball.radius;
                    ball.velocity.x = -ball.velocity.x;
                }

                if (ball.position.y - ball.radius <= 0.0f) {
                    ball.position.y = ball.radius;
                    ball.velocity.y = -ball.velocity.y;
                } else if (ball.position.y + ball.radius >= WINDOW_HEIGHT) {
                    ball.position.y = WINDOW_HEIGHT - ball.radius;
                    ball.velocity.y = -ball.velocity.y;
                }
            }
            BuildGrid(balls, grid);
            ResolveCollisions(balls, grid);

            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (showGrid) {
            for (int row = 0; row < GRID_ROWS; ++row) {
                for (int col = 0; col < GRID_COLS; ++col) {
                    const GridCell& cell = grid[row * GRID_COLS + col];

                    // 1. Draw grid cell outlines
                    DrawRectangleLinesEx(
                        Rectangle{ cell.position.x, cell.position.y, cell.width, cell.height },
                        1.0f,
                        Fade(DARKGRAY, 0.4f)
                    );

                    // 2. Render grid array indices (top-left)
                    DrawText(TextFormat("(%d,%d)", col, row), (int)cell.position.x + 3, (int)cell.position.y + 3, 10, DARKGRAY);

                    // 3. Render particle count in cell center (displays 0 if empty)
                    int count = (int)cell.ball_indices.size();
                    Color countColor = (count > 0) ? YELLOW : DARKGRAY;

                    DrawText(
                        TextFormat("%d", count),
                        (int)(cell.position.x + cell.width / 2.0f - 4.0f),
                        (int)(cell.position.y + cell.height / 2.0f - 6.0f),
                        14,
                        countColor
                    );
                }
            }
        }

        for (const Ball& ball : balls) {
            DrawCircleV(ball.position, ball.radius, ball.color);
        }

        DrawText(TextFormat("# OF SPACE presses %d", SPAWN_COUNT), 12, 20, 20, RAYWHITE);
        DrawText(TextFormat("PARTICLES %d", PARTICLE_COUNT), 12, 40, 20, RAYWHITE);
        DrawText("SPACE spawn   Q to toggle grid",
                 12, WINDOW_HEIGHT - 28, 18, Fade(RAYWHITE, 0.6f));

        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}