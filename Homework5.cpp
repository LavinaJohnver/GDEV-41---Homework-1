/*
    GDEV 41 - Homework 05: Entity-Component-System (EnTT) Asteroids

    TODO LIST
    [x] 1.  Raylib window 1280x720, 60 FPS target
    [x] 2.  entt::registry created; components defined as plain structs
    [x] 3.  SpawnSystem: spawn an asteroid every SPAWN_INTERVAL seconds
            somewhere inside the center spawn region
    [x] 4.  Asteroid = Position + Velocity + CircleShape (radius 20-50, default color)
    [x] 5.  MovementSystem: position += velocity * dt
    [x] 6.  BounceSystem: negate velocity on window edges
    [x] 7.  RenderSystem: draw every entity with Position + CircleShape
    [x] 8.  RollProperties(): 50% normal / 25% attracted / 25% repelled,
            recolor accordingly
    [x] 9.  MouseForceSystem: push velocity toward / away from the cursor
    [x] 10. RollProperties(): 35% chance -> add Lifetime (2-5 seconds)
    [x] 11. LifetimeSystem: count down, tag with Destroy when it hits 0
    [x] 12. RollProperties(): 10% chance -> add Splitter (2-5 children)
    [x] 13. DestructionSystem: for every Destroy-tagged entity, spawn its
            Splitter children at its position, THEN destroy it
    [x] 14. ClickSystem: left-click on an asteroid -> tag it with Destroy   (3pts w/ spawning)
    [x] 15. No asteroid cap (the registry grows as needed)
*/

#include <raylib.h>
#include "raymath.h"
#include "entt.hpp"
#include <vector>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
const int   WINDOW_WIDTH  = 1280;
const int   WINDOW_HEIGHT = 720;
const int   TARGET_FPS    = 60;

const float SPAWN_INTERVAL = 1.5f;     // seconds between spawns
const float SPAWN_REGION_W = 300.0f;   // size of center spawn region
const float SPAWN_REGION_H = 200.0f;

const int   MIN_RADIUS = 20;
const int   MAX_RADIUS = 50;
const int   MIN_SPEED  = 80;
const int   MAX_SPEED  = 250;

const float MOUSE_FORCE   = 400.0f;    // acceleration toward/away from cursor (px/s^2)
const float MAX_VELOCITY  = 500.0f;    // keeps attractwed asteroids from going insane

const Color COLOR_NORMAL    = LIGHTGRAY;
const Color COLOR_ATTRACTED = SKYBLUE;
const Color COLOR_REPELLED  = GREEN;


struct Position    { Vector2 value; };
struct Velocity    { Vector2 value; };
struct CircleShape { float radius; Color color; };

// +1 = attracted to the mouse, -1 = repelled by the mouse
struct MouseForce  { float direction; };

// Self-destruct countdown
struct Lifetime    { float timeLeft; };

// Spawns `count` asteroids in its place when destroyed
struct Splitter    { int count; };

// Tag component: "destroy me at the end of this frame"
struct Destroy     {};


static float RandomFloat(float min, float max) {
    return min + (max - min) * (GetRandomValue(0, 10000) / 10000.0f);
}

static bool RollChance(int percent) {
    return GetRandomValue(1, 100) <= percent;
}

// Decides the optional properties (part 3 of the specs) for a fresh asteroid.
static void RollProperties(entt::registry& registry, entt::entity e) {
    if (RollChance(50)) {
        float dir = RollChance(50) ? 1.0f : -1.0f;
        registry.emplace<MouseForce>(e, dir);
        registry.get<CircleShape>(e).color = (dir > 0) ? COLOR_ATTRACTED : COLOR_REPELLED;
    }

    if (RollChance(35)) {
        registry.emplace<Lifetime>(e, RandomFloat(2.0f, 5.0f));
    }

    if (RollChance(10)) {
        registry.emplace<Splitter>(e, GetRandomValue(2, 5));
    }
}

// Creates one asteroid entity at `pos`. Used by the spawner AND by splitters.
static entt::entity SpawnAsteroid(entt::registry& registry, Vector2 pos) {
    entt::entity e = registry.create();

    float angle = RandomFloat(0.0f, 2.0f * PI);
    float speed = (float)GetRandomValue(MIN_SPEED, MAX_SPEED);
    Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };

    registry.emplace<Position>(e, pos);
    registry.emplace<Velocity>(e, vel);
    registry.emplace<CircleShape>(e, (float)GetRandomValue(MIN_RADIUS, MAX_RADIUS), COLOR_NORMAL);

    RollProperties(registry, e);
    return e;
}

static void SpawnSystem(entt::registry& registry, float& spawnTimer, float dt) {
    spawnTimer += dt;
    while (spawnTimer >= SPAWN_INTERVAL) {
        spawnTimer -= SPAWN_INTERVAL;

        float cx = WINDOW_WIDTH  * 0.5f;
        float cy = WINDOW_HEIGHT * 0.5f;
        Vector2 pos = {
            RandomFloat(cx - SPAWN_REGION_W * 0.5f, cx + SPAWN_REGION_W * 0.5f),
            RandomFloat(cy - SPAWN_REGION_H * 0.5f, cy + SPAWN_REGION_H * 0.5f)
        };
        SpawnAsteroid(registry, pos);
    }
}

// TODO 9: Accelerate MouseForce asteroids toward (+1) or away from (-1) the cursor.
static void MouseForceSystem(entt::registry& registry, float dt) {
    Vector2 mouse = GetMousePosition();
    auto view = registry.view<Position, Velocity, MouseForce>();

    for (auto [e, pos, vel, force] : view.each()) {
        (void)e;
        Vector2 toMouse = Vector2Subtract(mouse, pos.value);

        if (Vector2Length(toMouse) < 1.0f) continue;

        Vector2 dir = Vector2Normalize(toMouse);
        vel.value = Vector2Add(vel.value, Vector2Scale(dir, force.direction * MOUSE_FORCE * dt));
        vel.value = Vector2ClampValue(vel.value, 0.0f, MAX_VELOCITY);
    }
}

static void MovementSystem(entt::registry& registry, float dt) {
    auto view = registry.view<Position, Velocity>();
    for (auto [e, pos, vel] : view.each()) {
        pos.value = Vector2Add(pos.value, Vector2Scale(vel.value, dt));
    }
}

// Negates velocity on the window edges. Position is 
// fixed to avoid asteroids getting stuck outside window area.
static void BounceSystem(entt::registry& registry) {
    auto view = registry.view<Position, Velocity, CircleShape>();
    for (auto [e, pos, vel, shape] : view.each()) {
        float r = shape.radius;
        if (pos.value.x - r < 0)             { pos.value.x = r;                 vel.value.x = -vel.value.x; }
        if (pos.value.x + r > WINDOW_WIDTH)  { pos.value.x = WINDOW_WIDTH - r;  vel.value.x = -vel.value.x; }
        if (pos.value.y - r < 0)             { pos.value.y = r;                 vel.value.y = -vel.value.y; }
        if (pos.value.y + r > WINDOW_HEIGHT) { pos.value.y = WINDOW_HEIGHT - r; vel.value.y = -vel.value.y; }
    }
}

static void LifetimeSystem(entt::registry& registry, float dt) {
    auto view = registry.view<Lifetime>();
    for (auto [e, life] : view.each()) {
        life.timeLeft -= dt;
        if (life.timeLeft <= 0.0f) {
            registry.emplace_or_replace<Destroy>(e);
        }
    }
}

// TODO 14: On left click, tag the asteroid under the cursor with Destroy.
static void ClickSystem(entt::registry& registry) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    Vector2 mouse = GetMousePosition();
    entt::entity hit = entt::null;
    float bestDist = 0.0f;

    auto view = registry.view<Position, CircleShape>();
    for (auto [e, pos, shape] : view.each()) {
        float d = Vector2Distance(mouse, pos.value);

        if (d <= shape.radius && (hit == entt::null || d < bestDist)) {
            hit = e;
            bestDist = d;
        }
    }

    if (hit != entt::null) {
        registry.emplace_or_replace<Destroy>(hit);
    }
}

// Destroys tagged stuff tagged w Destroy
static void DestructionSystem(entt::registry& registry) {
        auto view = registry.view<Destroy>();
        std::vector<entt::entity> toDestroy(view.begin(), view.end());
        
        for (auto e : toDestroy) {
            if (auto* splitter = registry.try_get<Splitter>(e)) {
                if (auto* pos = registry.try_get<Position>(e)) {
                    for (int i = 0; i < splitter->count; ++i) {
                        SpawnAsteroid(registry, pos->value);
                    }
                }
            }
            registry.destroy(e);
        }
    }

static void RenderSystem(entt::registry& registry) {
    auto view = registry.view<Position, CircleShape>();
    for (auto [e, pos, shape] : view.each()) {
        DrawCircleV(pos.value, shape.radius, shape.color);

        // (Optional) extra visuals for Lifetime / Splitter, e.g.:
        //   if (auto* life = registry.try_get<Lifetime>(e))
        //       DrawText(TextFormat("%.1f", life->timeLeft), ...);
        //   if (registry.all_of<Splitter>(e))
        //       DrawCircleLinesV(pos.value, shape.radius + 3, RED);
    }

    // Debug: center spawn region + entity count
    DrawRectangleLines((int)(WINDOW_WIDTH  * 0.5f - SPAWN_REGION_W * 0.5f),
                       (int)(WINDOW_HEIGHT * 0.5f - SPAWN_REGION_H * 0.5f),
                       (int)SPAWN_REGION_W, (int)SPAWN_REGION_H, DARKGRAY);
    DrawText(TextFormat("Asteroids: %d", (int)registry.view<Position>().size()), 10, 10, 20, RAYWHITE);
    DrawFPS(10, 35);
}


int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "GDEV 41 - Homework 05: ECS Asteroids");
    SetTargetFPS(TARGET_FPS);

    entt::registry registry;
    float spawnTimer = 0.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // --- Update (order matters) ---
        SpawnSystem(registry, spawnTimer, dt);
        MouseForceSystem(registry, dt);
        MovementSystem(registry, dt);
        BounceSystem(registry);
        LifetimeSystem(registry, dt);
        ClickSystem(registry);
        DestructionSystem(registry);   // always last: removes tagged entities

        // --- Draw ---
        BeginDrawing();
        ClearBackground(BLACK);
        RenderSystem(registry);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
