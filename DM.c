#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>  // Bibliothèque pour le son et le rendu graphique

#define DEFAULT_ROWS 20
#define DEFAULT_COLS 40
#define CELL_SIZE 20
#define DEFAULT_FPS 10
#define DEFAULT_ITERATIONS -1 // -1 signifie "infini"

// Structure représentant la grille
typedef struct {
    int rows;
    int cols;
    int **cells;
} Grid;

// Fonction pour créer une grille vide
Grid *create_grid(int rows, int cols) {
    Grid *grid = malloc(sizeof(Grid));
    grid->rows = rows;
    grid->cols = cols;
    grid->cells = malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        grid->cells[i] = malloc(cols * sizeof(int));
        memset(grid->cells[i], 0, cols * sizeof(int)); // Initialiser toutes les cellules à 0
    }
    return grid;
}

// Libérer la mémoire de la grille
void free_grid(Grid *grid) {
    for (int i = 0; i < grid->rows; i++) {
        free(grid->cells[i]);
    }
    free(grid->cells);
    free(grid);
}

// Initialiser la grille avec un motif de départ
void initialize_grid(Grid *grid, const char *pattern) {
    printf("Motif choisi : %s\n", pattern); // DEBUG : afficher le motif sélectionné

    int mid_row = grid->rows / 2;
    int mid_col = grid->cols / 2;

    if (strcmp(pattern, "oscillator") == 0) {
        grid->cells[mid_row][mid_col] = 1;
        grid->cells[mid_row][mid_col + 1] = 1;
        grid->cells[mid_row][mid_col - 1] = 1;
    } else if (strcmp(pattern, "glider") == 0) {
        grid->cells[mid_row][mid_col] = 1;
        grid->cells[mid_row - 1][mid_col + 1] = 1;
        grid->cells[mid_row - 2][mid_col - 1] = 1;
        grid->cells[mid_row - 2][mid_col] = 1;
        grid->cells[mid_row - 2][mid_col + 1] = 1;
    } else if (strcmp(pattern, "asterisk") == 0) {
        grid->cells[mid_row][mid_col] = 1;
    } else {
        printf("Motif inconnu '%s'. Utilisation de l'oscillator par défaut.\n", pattern);
        grid->cells[mid_row][mid_col] = 1;
        grid->cells[mid_row][mid_col + 1] = 1;
        grid->cells[mid_row][mid_col - 1] = 1;
    }
}

// Fonction pour compter les voisins vivants d'une cellule
int count_neighbors(const Grid *grid, int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue; // Ignorer la cellule elle-même
            int nx = x + i;
            int ny = y + j;
            if (nx >= 0 && nx < grid->rows && ny >= 0 && ny < grid->cols) {
                count += grid->cells[nx][ny];
            }
        }
    }
    return count;
}

// Mise à jour de la grille selon les règles du jeu
void update_grid(Grid *grid, int survive[9], int birth[9]) {
    int **new_cells = malloc(grid->rows * sizeof(int *));
    for (int i = 0; i < grid->rows; i++) {
        new_cells[i] = malloc(grid->cols * sizeof(int));
        for (int j = 0; j < grid->cols; j++) {
            int neighbors = count_neighbors(grid, i, j);
            new_cells[i][j] = (grid->cells[i][j] == 1) ? survive[neighbors] : birth[neighbors];
        }
    }

    for (int i = 0; i < grid->rows; i++) {
        memcpy(grid->cells[i], new_cells[i], grid->cols * sizeof(int));
        free(new_cells[i]);
    }
    free(new_cells);
}

// Fonction pour afficher la grille avec Raylib
void display_grid_raylib(const Grid *grid) {
    BeginDrawing();
    ClearBackground(BLACK);
    for (int i = 0; i < grid->rows; i++) {
        for (int j = 0; j < grid->cols; j++) {
            if (grid->cells[i][j]) {
                DrawRectangle(j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE, (Color){255, 105, 180, 255});
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
    int survive[9] = {0, 0, 1, 1, 0, 0, 0, 0, 0};
    int birth[9] = {0, 0, 1, 1, 0, 0, 0, 0, 0};

    // Lecture des arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--rows") == 0 && i + 1 < argc) rows = atoi(argv[++i]);
        else if (strcmp(argv[i], "--cols") == 0 && i + 1 < argc) cols = atoi(argv[++i]);
        else if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) fps = atoi(argv[++i]);
        else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) max_iterations = atoi(argv[++i]);
        else if (strcmp(argv[i], "--pattern") == 0 && i + 1 < argc) strncpy(pattern, argv[++i], sizeof(pattern));
    }

    Grid *grid = create_grid(rows, cols);
    initialize_grid(grid, pattern);

    int iteration = 0;
    bool paused = true;
    bool started = false;

    InitWindow(cols * CELL_SIZE, rows * CELL_SIZE, "L'expérience du Jeu de la Vie - Lost Sunset Riley");
    InitAudioDevice();  // Initialisation de l'audio
    Music music = LoadMusicStream("musiquedm.mp3");  // Charger la musique
    if (!IsAudioStreamReady(music.stream)) {
        printf("Erreur : Impossible de charger la musique.\n");
        return -1;  // Quitte le programme si la musique ne peut pas être chargée
    }

    PlayMusicStream(music);  // Démarrer la lecture de la musique
    SetTargetFPS(fps);

    while (!WindowShouldClose()) {
        UpdateMusicStream(music);  // Mettre à jour le flux audio
        if (!paused) update_grid(grid, survive, birth);
        display_grid_raylib(grid);
    }

    UnloadMusicStream(music);  // Libérer la mémoire associée à la musique
    CloseAudioDevice();        // Fermer le périphérique audio
    CloseWindow();
    free_grid(grid);
    return 0;
}


// Simulation configurable : pattern = glider, oscillator (par défaut sinon), asterik

/*
Commandes pour compiler et exécuter ce programme :

# Compilation
gcc -o DM DM.c -lraylib -L/Users/clara/Downloads/raylib-5.0_macos/lib -I/Users/clara/Downloads/raylib-5.0_macos/include -Wl,-rpath,/Users/clara/Downloads/raylib-5.0_macos/lib

# Définir DYLD_LIBRARY_PATH si nécessaire
export DYLD_LIBRARY_PATH=/Users/clara/Downloads/raylib-5.0_macos/lib:$DYLD_LIBRARY_PATH

# Exécution
./DM --rows 70 --cols 60 --fps 6 --iterations -1 --pattern glider

*/

