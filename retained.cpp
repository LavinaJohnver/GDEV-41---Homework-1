#include <raylib.h>
#include <raymath.h>

#include <iostream>
#include <functional>
#include <string>
#include <vector>

// Generic UI component
struct UIComponent
{
    // Rectangle reprsenting the bounds of the UI component
    Rectangle bounds;

    // Draws this particular UI component
    // Set as abstract so that child widgets will implement this for us
    virtual void Draw() = 0;

    // Handles a mouse click event
    // Set as abstract so that child widgets will implement this for us
    // Returns a boolean indicating whether this UI component successfully handled the event
    virtual bool HandleClick(Vector2 click_position) = 0;
};

// Generic UI component that can contain other UI components as children
struct UIContainer : public UIComponent
{
    std::vector<UIComponent*> children;

    // Adds a child to the container
    void AddChild(UIComponent* child)
    {
        children.push_back(child);
    }

    // Draw
    void Draw() override
    {
        // Since we are just a container for other widgets, we simply
        // call the draw function of other widgets.
        // This results in a pre-order traversal when we also draw child widgets that are also containers
        for (size_t i = 0; i < children.size(); ++i)
        {
            children[i]->Draw();
        }
    }

    // Handles a mouse click event
    // Returns a boolean indicating whether this UI component successfully handled the event
    bool HandleClick(Vector2 click_position) override
    {
        // Since we are just a container for other widgets, we call the HandleClick function of our child widgets
        // Since later children are drawn last, we do the opposite of draw where we start from the last child.
        // This results in a pre-order traversal but in the reverse order.
        for (size_t i = children.size(); i > 0; --i)
        {
            // If a child already handles the click event, we instantly return so no more child widgets handle the click
            if (children[i - 1]->HandleClick(click_position))
            {
                return true;
            }
        }

        return false;
    }
};

// Button widget
struct Button : public UIComponent
{
    // Text displayed by the button
    std::string text;
    std::function<void()> on_click;

    // Draw
    void Draw() override
    {
        DrawRectangleRec(bounds, GRAY);
        DrawText(text.c_str(), bounds.x, bounds.y, 14, BLACK);
    }

    // Handle mouse click
    // Returns a boolean indicating whether this UI component successfully handled the event
    bool HandleClick(Vector2 click_position) override
    {
        // Check if the mouse click position is within our bounds
        if (CheckCollisionPointRec(click_position, bounds))
        {
            if (on_click)
            {
                on_click();
            }
            else
            {
                std::cout << "Hello!" << std::endl;
            }
            return true;
        }

        return false;
    }
};

// Checkbox widget
struct CheckBox : public UIComponent
{
    // Whether the checkbox is checked or not
    bool checked = false;

    void Draw() override
    {
        DrawRectangleRec(bounds, GRAY);
        if (checked)
        {
            DrawText("X", bounds.x + 5, bounds.y + 5, 14, BLACK);
        }
    }

    // Handle mouse click
    // Returns a boolean indicating whether this UI component successfully handled the event
    bool HandleClick(Vector2 click_position) override
    {
        // Check if the mouse click position is within our bounds
        if (CheckCollisionPointRec(click_position, bounds))
        {
            checked = !checked;
            return true;
        }

        return false;
    }
};

// Text display widget
struct Label : public UIComponent
{
    // Text to be displayed
    std::string text;

    // Draw
    void Draw() override
    {
        DrawText(text.c_str(), bounds.x, bounds.y, 14, BLACK);
    }

    // Handle mouse click
    // Returns a boolean indicating whether this UI component successfully handled the event
    bool HandleClick(Vector2 click_position) override
    {
        // Always return false since we're not going to handle click events for this particular widget
        // (unless you have to)
        return false;
    }
};

// Struct to encapsulate our UI library
struct UILibrary
{
    // Root container
    UIContainer root_container;

    // Updates the current UI state
    void Update()
    {
        // If the left mouse button was released, we handle the click from the root container
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            root_container.HandleClick(GetMousePosition());
        }
    }

    // Draw
    void Draw()
    {
        root_container.Draw();
    }
};

int main()
{
    int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Retained Mode");
    SetTargetFPS(60);

    UILibrary ui_library;
    ui_library.root_container.bounds = { 10, 10, 600, 500 };

    Button button;
    button.text = "Hello!";
    button.bounds = { 120, 10, 80, 40 };
    ui_library.root_container.AddChild(&button);

    Button button2;
    button2.text = "Hi!";
    button2.bounds = { 210, 10, 80, 40 };
    ui_library.root_container.AddChild(&button2);

    Label label;
    label.text = "This is a label";
    label.bounds = { 10, 20, 100, 40 };
    ui_library.root_container.AddChild(&label);

    CheckBox checkbox;
    checkbox.bounds = { 10, 50, 20, 20 };
    ui_library.root_container.AddChild(&checkbox);
    Label checkbox_label;
    checkbox_label.text = "Lock Screensize";
    checkbox_label.bounds = { 40, 50, 100, 20 };
    ui_library.root_container.AddChild(&checkbox_label);

    Button setSizeButton1;
    setSizeButton1.text = "Set Size 800x600";
    setSizeButton1.bounds = { 10, 80, 150, 40 };
    setSizeButton1.on_click = [&checkbox]()
    {
        if (!checkbox.checked)
        {
            SetWindowSize(800, 600);
        }
        printf("Set Size 800x600\n");
    };
    ui_library.root_container.AddChild(&setSizeButton1);

    Button setSizeButton2;
    setSizeButton2.text = "Set Size 1280x720";
    setSizeButton2.bounds = { 10, 130, 150, 40 };
    setSizeButton2.on_click = [&checkbox]()
    {
        if (!checkbox.checked)
        {
            SetWindowSize(1280, 720);
        }
        printf("Set Size 1280x720n");
    };
    ui_library.root_container.AddChild(&setSizeButton2);

    Button setSizeButton3;
    setSizeButton3.text = "Set Size 1366x768";
    setSizeButton3.bounds = { 10, 180, 150, 40 };
    setSizeButton3.on_click = [&checkbox]()
    {
        if (!checkbox.checked)
        {
            SetWindowSize(1366, 768);
        }
        printf("Set Size 1366x768\n");
    };
    ui_library.root_container.AddChild(&setSizeButton3);

    while (!WindowShouldClose())
    {
        ui_library.Update();

        ClearBackground(WHITE);
        BeginDrawing();
        ui_library.Draw();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
