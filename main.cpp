#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>
#include <random>
#include <cctype>

#ifdef _WIN32
    #include <windows.h>
#endif

struct grid {
    int row;
    int col;
    std::string** grid_array;
    int player_row;
    int player_col;
    int enemy_row;
    int enemy_col;
    std::string player_symbol = "▲";

    // Construct a grid based on provided dimensions
    void construct_grid(int w, int h) {
        this->row = w;
        this->col = h;

        grid_array = new std::string*[row];
        for (int i = 0; i < row; ++i) {
            grid_array[i] = new std::string[col];
            for (int j = 0; j < col; ++j) {
                grid_array[i][j] = " ";
            }
        }
    }

    // Randomize initial player and enemy positions
    void spawn_position() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist_row(0, row - 1);
        std::uniform_int_distribution<int> dist_col(0, col - 1);

        player_row = dist_row(rng);
        player_col = dist_col(rng);
        grid_array[player_row][player_col] = player_symbol;

        do {
            enemy_row = dist_row(rng);
            enemy_col = dist_col(rng);
        } while (enemy_row == player_row && enemy_col == player_col);
        grid_array[enemy_row][enemy_col] = "■";
    }

    // Move player based on input direction, update grid and player symbol
    bool move_player(const std::string& input) {
        std::string dir = input;
        for (size_t i = 0; i < dir.size(); ++i) {
            dir[i] = std::tolower(dir[i]);
        }

        int new_row = player_row;
        int new_col = player_col;
        std::string new_symbol = player_symbol;

        if (dir == "north" || dir == "n") {
            new_row -= 1;
            new_symbol = "▲";
        } else if (dir == "south" || dir == "s") {
            new_row += 1;
            new_symbol = "▼";
        } else if (dir == "east" || dir == "e") {
            new_col += 1;
            new_symbol = "►";
        } else if (dir == "west" || dir == "w") {
            new_col -= 1;
            new_symbol = "◄";
        } else {
            return false; // input not registered
        }

        if (new_row < 0 || new_row >= row || new_col < 0 || new_col >= col) {
            return false; // would move off the grid
        }

        if (player_row == enemy_row && player_col == enemy_col) {
            grid_array[player_row][player_col] = "■";
        } else {
            grid_array[player_row][player_col] = " ";
        }
        player_row = new_row;
        player_col = new_col;
        player_symbol = new_symbol;
        grid_array[player_row][player_col] = player_symbol;
        return true;
    }

    // Check if player in same cell
    bool attack() {
        if (player_row == enemy_row && player_col == enemy_col) {
            return true;
        }
        return false;
    }

    // Function to build horizontal lines with specific characters for edges, junctions, and fillers
    std::string build_line(std::string& left, std::string& middle,
                            std::string& right, std::string& filler) {
        std::string line = left;
        for (int i = 0; i < col; ++i) {
            line += filler + filler + filler;
            line += (i == col - 1) ? right : middle;
        }
        return line;
    }

    // Render the grid with borders and symbols
    void render_grid() {
        std::string top_l = "┌", top_m = "┬", top_r = "┐";
        std::string mid_l = "├", mid_m = "┼", mid_r = "┤";
        std::string bot_l = "└", bot_m = "┴", bot_r = "┘";
        std::string x_fill = "─", y_fill = "│";

        std::string top_line = build_line(top_l, top_m, top_r, x_fill);
        std::string mid_line = build_line(mid_l, mid_m, mid_r, x_fill);
        std::string bot_line = build_line(bot_l, bot_m, bot_r, x_fill);

        std::cout << top_line << std::endl;

        for (int i = 0; i < row; ++i) {
            std::cout << y_fill;

            for (int j = 0; j < col; ++j) {
                std::string cell = grid_array[i][j].empty() ? " " : grid_array[i][j]; // If cell is empty, display space
                std::cout << " " << cell << " " << y_fill;
            }
            std::cout << std::endl;

            std::cout << (i == row - 1 ? bot_line : mid_line) << std::endl;
        }
    }

    // Deallocating memory
    ~grid() {
        for (int i = 0; i < row; ++i) {
            delete[] grid_array[i];   // free each row
        }
        delete[] grid_array;          // free the array of row pointers
    }
};

void load_settings(const std::string& filepath, int& outRow, int& outCol) {
    std::ifstream file(filepath);

    if(!file.is_open()) {
        std::cerr << "Error: Could not open settings.txt!" << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if(line.empty() || line[0] == '#') { // skip empty lines + comments
            continue;
        }

        std::istringstream line_stream(line);
        std::string key, value_str;

        if (std::getline(line_stream, key, '=') && std::getline(line_stream, value_str)) {
            try {
                int value = std::stoi(value_str);
                if (key == "row") {
                    outRow = value;
                } else if (key == "col") {
                    outCol = value;
                }
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid value for " << key << ": " << value_str << " in settings.txt\n";
            }
        }
    }
    file.close();
}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif

    int settings_row = 20; // set default in case settings.txt is wrong
    int settings_col = 10;

    load_settings("settings.txt", settings_row, settings_col);

    grid g;
    g.construct_grid(settings_col, settings_row);
    g.spawn_position();

    std::cout   << "========================= TRIANGLES VS SQUARES =========================\n"
                << "These squares have invaded our territory! We are deploying you to eliminate them!\n\n\n"
                << "Type \"attack\" or \"a\" to attack the square if you both are in the same cell!\n"
                << "Move with north/south/east/west (or n/s/e/w). Type \"exit\" to quit.\n";

    g.render_grid();

    std::string input;
    while (std::getline(std::cin, input)) {
        std::string lowered = input;
        for (size_t i = 0; i < lowered.size(); ++i) {
            lowered[i] = std::tolower(lowered[i]);
        }

        if (lowered == "exit") {
            break;
        }

        if (lowered == "attack" || lowered == "a") {
            if (g.attack()) {
                clear_screen();
                g.render_grid();
                std::cout << "Triangles triumph!\n";
                break;
            } else {
                std::cout << "Nothing happens...\n";
            }
            continue;
        }

        if (g.move_player(input)) {
            clear_screen();
            std::cout << "Type \"attack\" or \"a\" to attack the square if you both are in the same cell!\n"
                      << "Move with north/south/east/west (or n/s/e/w). Type \"exit\" to quit.\n";
            g.render_grid();
        }
        // Anything else (including unrecognized input) isn't registered
    }
    return 0;
}