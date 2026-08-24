#include <iostream>
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

        grid_array = new std::string*[row];       // allocate array of row pointers
        for (int i = 0; i < row; ++i) {
            grid_array[i] = new std::string[col];   // allocate each row
            for (int j = 0; j < col; ++j) {
                grid_array[i][j] = " ";                // initialize cell
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

    // Deallocate memory stuff
    ~grid() {
        for (int i = 0; i < row; ++i) {
            delete[] grid_array[i];   // free each row
        }
        delete[] grid_array;          // free the array of row pointers
    }
};

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif

    grid g;
    g.construct_grid(10, 20);
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