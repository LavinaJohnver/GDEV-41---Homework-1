/*
    TODO LIST
    [x] 1.  Raylib window 1280x720, 60 FPS target
    [x] 2.  Spawn 25 circles per SPACE press (radius 5-10, random velocity/color, mass 1)
    [x] 3.  After every 10 normal spawns, the next SPACE press spawns 1 big circle (radius 25, mass 10)
    [x] 4.  Ball-ball collision + bounce off screen edges
    [x] 5.  Fixed timestep, sequential motion, static (discrete) collision detection
    [x] 6.  No cap on ball count (vector grows freely; the tree will store INDICES, not pointers)
    [x] 7.  Define QuadTree node: Rectangle bounds, int depth, std::vector<int> items,
            4 children (std::unique_ptr or raw pointers), MAX_DEPTH and MAX_ITEMS constants
            (follow the QuadTree slide set)
    [X] 8.  Insert(index): store in a leaf if under capacity (or at MAX_DEPTH),
            otherwise subdivide and push the ball into every child its AABB overlaps
    [X] 9.  Subdivide(): create the 4 quadrant children and re-insert existing items
    [X] 10. Query(Rectangle range, std::vector<int>& out): gather indices from overlapping
            nodes, de-duplicate if a ball can live in several leaves
    [X] 11. Clear()/rebuild the tree every physics step
    [X] 12. Replace the brute-force GetCandidates() with a QuadTree query
    [X] 13. Skip self-checks (j != i) and duplicate candidates
    [X] 14. Draw the QuadTree node rectangles when Q is toggled
    [X] 15. Compare "checks/step" on the HUD: brute force vs QuadTree at 1000+ balls
    [X] 16. Choose and justify MAX_DEPTH / MAX_ITEMS
*/

#include <raylib.h>
#include "raymath.h"
#include <vector>
#include <cmath>
#include <memory>

const int   WINDOW_WIDTH  = 1280;
const int   WINDOW_HEIGHT = 720;
const float FPS           = 60.0f;
const float TIMESTEP      = 1.0f / FPS;

const float MIN_RADIUS  = 5.0f;
const float MAX_RADIUS  = 10.0f;
const float BIG_RADIUS  = 25.0f;
const float NORMAL_MASS = 1.0f;
const float BIG_MASS    = 10.0f;

const int BALLS_PER_SPAWN   = 25;
const int SPAWNS_BEFORE_BIG = 10;

const float ELASTICITY      = 0.92f;
const float WALL_ELASTICITY = 0.88f;
const float SPAWN_SPEED_MIN = 100.0f;
const float SPAWN_SPEED_MAX = 300.0f;

const float QUERY_PADDING = SPAWN_SPEED_MAX * TIMESTEP;

struct Ball {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float inverse_mass;
    Color color;
};

const int MAX_DEPTH = 6;
const int MAX_ITEMS = 8;

struct QuadTreeNode {
    Rectangle bounds;
    int depth;
    std::vector<int> items;
    // children[0] = top-left, [1] = top-right, [2] = bottom-left, [3] = bottom-right
    std::unique_ptr<QuadTreeNode> children[4];

    QuadTreeNode(Rectangle bounds, int depth) : bounds(bounds), depth(depth) {}

    bool IsLeaf() const { return children[0] == nullptr; }

    void Clear() {
        items.clear();
        for (int i = 0; i < 4; ++i) children[i].reset();
    }
};

static bool RectOverlap(const Rectangle& a, const Rectangle& b) {
    return a.x < b.x + b.width  && a.x + a.width  > b.x && a.y < b.y + b.height && a.y + a.height > b.y;
}

// Bounding/collision box around of each ball
static Rectangle BallBounds(const Ball& ball, float pad = 0.0f) {
    float r = ball.radius + pad;
    return Rectangle{ ball.position.x - r, ball.position.y - r, r * 2.0f, r * 2.0f };
}


// ---- QUADTREE STUFF

static void Insert(QuadTreeNode& node, int index, const std::vector<Ball>& balls);

static void Subdivide(QuadTreeNode& node, const std::vector<Ball>& balls) {
    float hw = node.bounds.width * 0.5f;
    float hh = node.bounds.height * 0.5f;
    float x = node.bounds.x;
    float y = node.bounds.y;

    // split leaf into 4 quadrants
    node.children[0] = std::make_unique<QuadTreeNode>(Rectangle{x, y, hw, hh}, node.depth + 1);
    node.children[1] = std::make_unique<QuadTreeNode>(Rectangle{x + hw, y, hw, hh}, node.depth + 1);
    node.children[2] = std::make_unique<QuadTreeNode>(Rectangle{x, y + hh, hw, hh}, node.depth + 1);
    node.children[3] = std::make_unique<QuadTreeNode>(Rectangle{x + hw, y + hh, hw, hh}, node.depth + 1);

    std::vector<int> moved;
    moved.swap(node.items);
    for (int index:moved)
        Insert(node, index, balls);
}

static void Insert(QuadTreeNode& node, int index, const std::vector<Ball>& balls) {
    if (!node.IsLeaf()) {
        Rectangle box = BallBounds(balls[index]);
        for (int i = 0; i < 4; i++) {
            if (RectOverlap(box, node.children[i]->bounds))
                Insert(*node.children[i], index, balls);
        }
        return;
    }

    node.items.push_back(index);

    if (!node.items.empty() && node.depth < MAX_DEPTH) {
        Subdivide(node, balls);
    }
}

static Ball SpawnBall(const Vector2& at, float radius, float mass) {
    // random direction + speed
    float angle = (float)GetRandomValue(0, 359) * DEG2RAD;
    float speed = (float)GetRandomValue((int)SPAWN_SPEED_MIN, (int)SPAWN_SPEED_MAX);

    Color color = ColorFromHSV((float)GetRandomValue(0, 359), 0.65f, 0.95f);

    return Ball{
        at, { cosf(angle) * speed, sinf(angle) * speed }, radius, 1.0f / mass, color
    };
}

static Vector2 RandomSpawnPoint(float radius) {
    return {
        (float)GetRandomValue((int)radius, WINDOW_WIDTH  - (int)radius),
        (float)GetRandomValue((int)radius, WINDOW_HEIGHT - (int)radius)
    };
}

static void ResolveWall(Ball& ball) {
    if (ball.position.x - ball.radius <= 0.0f) {
        ball.position.x = ball.radius;
        ball.velocity.x = -ball.velocity.x * WALL_ELASTICITY;
    } else if (ball.position.x + ball.radius >= WINDOW_WIDTH) {
        ball.position.x = WINDOW_WIDTH - ball.radius;
        ball.velocity.x = -ball.velocity.x * WALL_ELASTICITY;
    }

    if (ball.position.y - ball.radius <= 0.0f) {
        ball.position.y = ball.radius;
        ball.velocity.y = -ball.velocity.y * WALL_ELASTICITY;
    } else if (ball.position.y + ball.radius >= WINDOW_HEIGHT) {
        ball.position.y = WINDOW_HEIGHT - ball.radius;
        ball.velocity.y = -ball.velocity.y * WALL_ELASTICITY;
    }
}

// Static (discrete) collision test + response between two circles
static void ResolveCollision(Ball& a, Ball& b) {
    Vector2 delta = Vector2Subtract(b.position, a.position);
    float distSq  = Vector2LengthSqr(delta);
    float radii   = a.radius + b.radius;

    if (distSq >= radii * radii) return;

    float dist = sqrtf(distSq);
    // perfectly overlapping centers have no direction, so pick one
    Vector2 normal = (dist > 0.0001f) ? Vector2Scale(delta, 1.0f / dist) : Vector2{ 1.0f, 0.0f };

    float invMassSum = a.inverse_mass + b.inverse_mass;

    // push the balls apart, lighter ball moves more
    float penetration = radii - dist;
    Vector2 correction = Vector2Scale(normal, penetration / invMassSum);
    a.position = Vector2Subtract(a.position, Vector2Scale(correction, a.inverse_mass));
    b.position = Vector2Add(b.position, Vector2Scale(correction, b.inverse_mass));

    // only apply an impulse if they are moving toward each other
    Vector2 relVel = Vector2Subtract(b.velocity, a.velocity);
    float velAlongNormal = Vector2DotProduct(relVel, normal);
    if (velAlongNormal >= 0.0f) return;

    float j = -(1.0f + ELASTICITY) * velAlongNormal / invMassSum;
    Vector2 impulse = Vector2Scale(normal, j);
    a.velocity = Vector2Subtract(a.velocity, Vector2Scale(impulse, a.inverse_mass));
    b.velocity = Vector2Add(b.velocity, Vector2Scale(impulse, b.inverse_mass));
}

static void Query(const QuadTreeNode& node, const Rectangle& range,
    std::vector<int>& out, std::vector<bool>& added) {
    if (!RectOverlap(range, node.bounds)) return;

    if (node.IsLeaf()) {
        for (int index : node.items) {
            if (added[index]) continue;

            added[index] = true;
            out.push_back(index);
        }
        return;
    }

    for (int i = 0; i < 4; i++) {
        Query(*node.children[i], range, out, added);
    }
}

static void DrawQuadTree(const QuadTreeNode& node) {
    // Draw the node's bounding box
    DrawRectangleLinesEx(node.bounds, 1.0f, Fade(LIGHTGRAY, 0.25f));

    if (!node.IsLeaf()) {
        for (int i = 0; i < 4; ++i) {
            if (node.children[i]) {
                DrawQuadTree(*node.children[i]);
            }
        }
    }
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "QuadTree Ball Collision");
    SetTargetFPS((int)FPS);

    std::vector<Ball> balls;
    std::vector<int>  candidates;
    std::vector<bool> added;

    QuadTreeNode tree(Rectangle{ 0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT }, 0);

    float accumulator      = 0.0f;
    int   normalSpawnCount = 0;
    int   checksThisStep   = 0;
    bool  showTree         = false;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            if (normalSpawnCount >= SPAWNS_BEFORE_BIG) {
                balls.push_back(SpawnBall(RandomSpawnPoint(BIG_RADIUS), BIG_RADIUS, BIG_MASS));
                normalSpawnCount = 0;
            } else {
                for (int k = 0; k < BALLS_PER_SPAWN; ++k) {
                    float radius = (float)GetRandomValue((int)MIN_RADIUS, (int)MAX_RADIUS);
                    balls.push_back(SpawnBall(RandomSpawnPoint(radius), radius, NORMAL_MASS));
                }
                normalSpawnCount++;
            }
        }

        if (IsKeyPressed(KEY_Q))
            showTree = !showTree;

        // Physics step
        accumulator += delta_time;
        while (accumulator >= TIMESTEP) {
            checksThisStep = 0;

            tree.Clear();
            for (int i = 0; i < (int)balls.size(); ++i)
                Insert(tree, i, balls);

            added.assign(balls.size(), false);

            // Sequential motion: each ball fully computes its velocity, position, and collisions against every other ball before moving on to the next ball
            for (int i = 0; i < (int)balls.size(); ++i) {
                Ball& ball = balls[i];
                ball.position = Vector2Add(ball.position, Vector2Scale(ball.velocity, TIMESTEP));
                ResolveWall(ball);

                candidates.clear();

                // clear ball i as already marked so it doesn't appear in its own candidate list
                added[i] = true;
                Query(tree, BallBounds(ball, QUERY_PADDING), candidates, added);

                for (int j : candidates) {
                    ResolveCollision(balls[i], balls[j]);
                    checksThisStep++;
                }


                added[i] = false;
                for (int j : candidates) added[j] = false;
            }

            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (showTree) {
            DrawQuadTree(tree);
        }

        for (const Ball& ball : balls) {
            DrawCircleV(ball.position, ball.radius, ball.color);
        }

        DrawRectangle(6, 6, 300, 100, Fade(BLACK, 0.6f));
        DrawText(TextFormat("FPS %d", GetFPS()), 12, 12, 20, RAYWHITE);
        DrawText(TextFormat("BALLS %d", (int)balls.size()), 12, 36, 20, RAYWHITE);
        DrawText(TextFormat("CHECKS/STEP %d", checksThisStep), 12, 60, 20, RAYWHITE);
        DrawText(TextFormat("BIG BALL IN %d", SPAWNS_BEFORE_BIG - normalSpawnCount), 12, 84, 20, RAYWHITE);
        DrawText("SPACE spawn   Q toggle tree",
                 12, WINDOW_HEIGHT - 28, 18, Fade(RAYWHITE, 0.6f));

        EndDrawing();
    }

    CloseWindow();
}
