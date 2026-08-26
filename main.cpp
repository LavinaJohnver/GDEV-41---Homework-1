#include <iostream>
#include <cmath>
#include <string>
#include <random>

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

    void spawn_position() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist_row(0, row - 1);
        std::uniform_int_distribution<int> dist_col(0, col - 1);

        int player_row = dist_row(rng);
        int player_col = dist_col(rng);
        grid_array[player_row][player_col] = "P"; // Spawns player

        int enemy_row, enemy_col;
        do {
            enemy_row = dist_row(rng);
            enemy_col = dist_col(rng);
        } while (enemy_row == player_row && enemy_col == player_col); // Ensure enemy doesn't spawn on player
        grid_array[enemy_row][enemy_col] = "E"; // Spawns enemy
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
    g.construct_grid(3, 10);
    g.spawn_position();
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