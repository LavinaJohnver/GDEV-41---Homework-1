#include <raylib.h>
#include "raymath.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

// Struct to encapsulate our UI library
struct UiLibrary
{
    // ID of the currently hot widget
    int hot = -1;

    // ID of the currently active widget
    int active = -1;

    // Creates a button with the specified text and bounds
    // Returns true if this button was clicked in this frame
    bool Button(int id, const std::string& text, const Rectangle& bounds)
    {
        bool result = false;

        // If the mouse cursor is hovering within our boundaries
        if (CheckCollisionPointRec(GetMousePosition(), bounds))
        {
            // Set this widget to be the hot widget
            hot = id;
        }
        // If the mouse cursor is not on top of this widget, and this widget
        // was previously the hot widget, set the hot widget to -1
        // We check if this widget was the previously hot widget since there is a possibility that
        // the hot widget is now a different widget, and we don't want to overwrite that with -1
        else if (hot == id) hot = -1;

        // If this button is the currently active widget, that means
        // the user is currently interacting with this widget
        if (id == active)
        {
            // SetWindowSize(int width, int height)

            // If the user released the mouse button while we are active,
            // register as a click
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                // Make sure that we are still the hot widget
                // (User might have dragged mouse to another widget, in which case we will not count as a button click)
                if (id == hot) result = true;

                // Set ourselves to not be active anymore
                active = -1;
            }
        }

        // If we are currently the hot widget and if the user pressed the 
        // left mouse button, that means the user started interacting with 
        // this widget, so we set this widget as active
        else if (id == hot && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            active = id;
        }
        
        Color ButtonColor = DARKGRAY;    // resting: visible against the black background
        Color TextColor = RAYWHITE;

        if (active == id && hot == id) // Button is held down
        { 
            ButtonColor = WHITE; 
            TextColor = BLACK; 
        }
        else if (hot == id) ButtonColor = LIGHTGRAY; // Button is hovered over

        DrawRectangleRec(bounds, ButtonColor);
        DrawText(text.c_str(), (int)bounds.x + 8, (int)bounds.y + 13, 14, TextColor);

        return result;
    }

    // Checkbox element
    bool CheckBox(int id, const std::string& text, const Rectangle& bounds, bool& isChecked)
    {
        // Copy of hover from button
        if (CheckCollisionPointRec(GetMousePosition(), bounds)) hot = id;
        else if (hot == id) hot = -1;

        // Also identical to Button, except a completed click 
        // reverses the connected variable's state instead of 
        // returning true
        if (id == active)
        {
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                if (id == hot) isChecked = !isChecked;
                active = -1;
            }
        }
        else if (id == hot && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            active = id;
        }

        Color BoxColor = DARKGRAY;
        Color TextColor = RAYWHITE;

        if (active == id && hot == id) BoxColor = GRAY; // Held down
        else if (hot == id) BoxColor = LIGHTGRAY;  // Hovered over

        // The box is a square at the left end, sized to the widget's height
        float boxSize = bounds.height;
        Rectangle box = { bounds.x, bounds.y, boxSize, boxSize };

        DrawRectangleRec(box, BoxColor);
        DrawRectangleLinesEx(box, 2, RAYWHITE);

        // The checkmark is instead a smaller square
        if (isChecked)
        {
            float pad = boxSize * 0.2f;
            DrawRectangleRec({ box.x + pad, box.y + pad, boxSize - 2 * pad, boxSize - 2 * pad }, LIME);
        }

        DrawText(text.c_str(), (int)(bounds.x + boxSize + 8), (int)(bounds.y + 4), 14, TextColor);

        return isChecked;
    }
};

int main()
{
    int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Immediate Mode");
    SetTargetFPS(60);

    UiLibrary uiLibrary;

    bool lockScreenSize = false;

    while (!WindowShouldClose())
    {
        ClearBackground(BLACK);
        BeginDrawing();

        DrawText(TextFormat("Resolution:"),10,24,14,RAYWHITE);

        if (uiLibrary.Button(0, "800 x 600", { 90, 10, 100, 40 }))
        {
            std::cout << "Hello!" << std::endl;
        }
        if (uiLibrary.Button(1, "1280 x 720", { 200, 10, 100, 40 }))
        {
            std::cout << "Hi!" << std::endl;
        }
        if (uiLibrary.Button(2, "1366 x 768", { 310, 10, 100, 40 }))
        {
            std::cout << "Hola!" << std::endl;
        }

        if (uiLibrary.CheckBox(3, "Lock screen size", { 10, 60, 200, 24 }, lockScreenSize))
        {
            std::cout << "Screen size locked!" << std::endl;
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
