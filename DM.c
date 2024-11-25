#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>

#define DEFAULT_ROWS 20
#define DEFAULT_COLS 40
#define CELL_SIZE 20
#define DEFAULT_FPS 10
#define DEFAULT_ITERATIONS -1 // -1 signifie "infini"

typedef struct {
    int rows;
    int cols;
    int **cells;
} Grid;

// Fonction pour créer une grille
Grid *create_grid(int rows, int cols) {
    Grid *grid = malloc(sizeof(Grid));
    grid->rows = rows;
    grid->cols = cols;
    grid->cells = malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        grid->cells[i] = malloc(cols * sizeof(int));
        memset(grid->cells[i], 0, cols * sizeof(int));
    }
    return grid;
}

// Libération de mémoire
void free_grid(Grid *grid) {
    for (int i = 0; i < grid->rows; i++) {
        free(grid->cells[i]);
    }
    free(grid->cells);
    free(grid);
}

// Initialisation avec un motif de départ
void initialize_grid(Grid *grid, const char *pattern) {
    if (strcmp(pattern, "oscillator") == 0) {
        int mid_row = grid->rows / 2;
        int mid_col = grid->cols / 2;
        grid->cells[mid_row][mid_col] = 1;
        grid->cells[mid_row][mid_col + 1] = 1;
        grid->cells[mid_row][mid_col - 1] = 1;
    } else if (strcmp(pattern, "glider") == 0) {
        int mid_row = grid->rows / 2;
        int mid_col = grid->cols / 2;
        grid->cells[mid_row][mid_col] = 1;
        grid->cells[mid_row - 1][mid_col + 1] = 1;
        grid->cells[mid_row - 2][mid_col - 1] = 1;
        grid->cells[mid_row - 2][mid_col] = 1;
        grid->cells[mid_row - 2][mid_col + 1] = 1;
    } else if (strcmp(pattern, "asterisk") == 0) {
        int mid_row = grid->rows / 2;
        int mid_col = grid->cols / 2;
        grid->cells[mid_row][mid_col] = 1;
    } else {
        printf("Motif non reconnu. Utilisation de l'oscillateur par défaut.\n");
        initialize_grid(grid, "oscillator");
    }
}

// Compte les voisins vivants
int count_neighbors(const Grid *grid, int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            int nx = x + i;
            int ny = y + j;
            if (nx >= 0 && nx < grid->rows && ny >= 0 && ny < grid->cols) {
                count += grid->cells[nx][ny];
            }
        }
    }
    return count;
}

// Mise à jour de la grille
void update_grid(Grid *grid, int survive[9], int birth[9]) {
    int **new_cells = malloc(grid->rows * sizeof(int *));
    for (int i = 0; i < grid->rows; i++) {
        new_cells[i] = malloc(grid->cols * sizeof(int));
        for (int j = 0; j < grid->cols; j++) {
            int neighbors = count_neighbors(grid, i, j);
            if (grid->cells[i][j] == 1) {
                new_cells[i][j] = survive[neighbors];
            } else {
                new_cells[i][j] = birth[neighbors];
            }
        }
    }
    for (int i = 0; i < grid->rows; i++) {
        memcpy(grid->cells[i], new_cells[i], grid->cols * sizeof(int));
        free(new_cells[i]);
    }
    free(new_cells);
}

// Affichage de la grille avec Raylib
void display_grid_raylib(const Grid *grid) {
    BeginDrawing();
    ClearBackground(BLACK);
    for (int i = 0; i < grid->rows; i++) {
        for (int j = 0; j < grid->cols; j++) {
            if (grid->cells[i][j]) {
                DrawRectangle(j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE, GREEN);
            } else {
                DrawRectangleLines(j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE, DARKGRAY);
            }
        }
    }
    EndDrawing();
}

int main(int argc, char *argv[]) {
    int rows = DEFAULT_ROWS;
    int cols = DEFAULT_COLS;
    int fps = DEFAULT_FPS;
    int max_iterations = DEFAULT_ITERATIONS;
    char pattern[20] = "oscillator";
    int survive[9] = {0, 0, 1, 1, 0, 0, 0, 0, 0}; // Survie par défaut : 2 ou 3 voisins
    int birth[9] = {0, 0, 0, 1, 0, 0, 0, 0, 0};   // Naissance par défaut : 3 voisins

    // Lecture des arguments en ligne de commande
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--rows") == 0 && i + 1 < argc) rows = atoi(argv[++i]);
        else if (strcmp(argv[i], "--cols") == 0 && i + 1 < argc) cols = atoi(argv[++i]);
        else if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) fps = atoi(argv[++i]);
        else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) max_iterations = atoi(argv[++i]);
        else if (strcmp(argv[i], "--pattern") == 0 && i + 1 < argc) strcpy(pattern, argv[++i]);
        else if (strcmp(argv[i], "--survive") == 0 && i + 9 < argc) {
            for (int j = 0; j < 9; j++) survive[j] = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--birth") == 0 && i + 9 < argc) {
            for (int j = 0; j < 9; j++) birth[j] = atoi(argv[++i]);
        }
    }

    Grid *grid = create_grid(rows, cols);
    initialize_grid(grid, pattern);

    int iteration = 0;

    // Initialisation de Raylib
    int screenWidth = cols * CELL_SIZE;
    int screenHeight = rows * CELL_SIZE;
    InitWindow(screenWidth, screenHeight, "Jeu de la Vie - Conway");
    SetTargetFPS(fps);

    while (!WindowShouldClose() && (max_iterations == -1 || iteration < max_iterations)) {
        update_grid(grid, survive, birth);
        display_grid_raylib(grid);
        iteration++;
    }

    CloseWindow();
    free_grid(grid);

    printf("Simulation terminée.\n");
    return 0;
}
