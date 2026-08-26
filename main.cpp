#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#endif

struct grid {
    int row;
    int col;
    std::string** grid_array;

    void construct_grid(int w, int h) {
        this->row = w;
        this->col = h;

        grid_array = new std::string*[row];         // allocate array of row pointers
        for (int i = 0; i < row; ++i) {
            grid_array[i] = new std::string[col];   // allocate each row
            for (int j = 0; j < col; ++j) {
                grid_array[i][j] = " ";             // populate cells with spaces
            }
        }
    }

    void render_grid() {
        std::string border(col * 4 + 1, '-');

        for (int i = 0; i < row; ++i) {
            std::cout << border << std::endl;

            for (int j = 0; j < col; ++j) {
                std::string cell = grid_array[i][j].empty() ? " " : grid_array[i][j]; // If cell is empty, display space
                std::cout << "| " << cell << " ";
            }
            std::cout << "|" << std::endl;
        }
        std::cout << border << std::endl;
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
    std::ifstream file("settings.txt");

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

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif

    int settings_row = 20; // set default in case settings.txt is wrong
    int settings_col = 10;

    load_settings("settings.txt", settings_row, settings_col);

    grid g;
    g.construct_grid(settings_col, settings_row);
    g.render_grid();

    // GRID VISUALIZATION FOR LATA
    // What grid should look like maybe, 
    // arrows are for player, X is for enemy
    //
    // std::cout << "┌───┬───┬───┐" << std::endl;
    // std::cout << "│   │ ▲ │   │" << std::endl;
    // std::cout << "├───┼───┼───┤" << std::endl;
    // std::cout << "│ ◄ │ X │ ► │" << std::endl;
    // std::cout << "├───┼───┼───┤" << std::endl;
    // std::cout << "│   │ ▼ │   │" << std::endl;
    // std::cout << "└───┴───┴───┘" << std::endl;
    return 0;
}