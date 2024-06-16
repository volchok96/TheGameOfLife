#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LIFE "*"
#define DEAD " "
#define MAXSPEED 2.0
#define MINSPEED 0.1
#define ITERSPEED 0.1
#define STARTSPEED 100000

void game_loop();  // main game loop
void print_menu();
void check_input(int *user_command);
void choose_option(int mode);
void game(int mode);
int alloc_memory(char ***matrix);
void free_memory(char **matrix);
void choose_option(int mode);
void print_field(char **matrix);
int update_field(char ***matrix, char ***buff);
void output_field(char **matrix, WINDOW *win);
int count_alive_cells(char **matrix, int i, int j);
char cell_update(char cell, int count, int *change_flag);
void change_speed(char button, float *speed);

int main() {
    game_loop();
    return 0;
}

void game_loop() {
    int command = 0;
    print_menu();
    check_input(&command);
    game(command);
}

void print_menu() {
    printf("Briefly about the game...");
    printf("Game of Life.\n");
    printf("The setting of the game is a grid divided into cells.");
    printf(
        "Each cell on this surface has eight neighbors surrounding it and can be in two states: alive or "
        "dead.");
    printf("The game ends if there is no alive cells left on the field.\n\n");
    printf("Now insert a number from 1 to 5 to choose a game mode: ");
    printf("1 - some card");
    printf("2 - another card");
    printf("3 - yet another card");
    printf("4 - next card");
    printf("5 - the last card, but not exactly");
}

void check_input(int *user_command) {
    int check = 0;
    while (!check) {
        printf("Enter command: ");
        check = scanf("%d", user_command);
        if (!check) {
            printf("Input error, please try again.");
            getchar();
        } else if (*user_command < 1 || *user_command > 6) {
            printf("You are probably beyond our capabilities.");
            check = 0;
            stdin = freopen("/dev/tty", "r", stdin);
        }
    }
}

void choose_option(int mode) {
    switch (mode) {
        case 1:
            stdin = freopen("./maps/map_1.txt", "r", stdin);
            break;
        case 2:
            stdin = freopen("./maps/map_2.txt", "r", stdin);
            break;
        case 3:
            stdin = freopen("./maps/map_3.txt", "r", stdin);
            break;
        case 4:
            stdin = freopen("./maps/map_4.txt", "r", stdin);
            break;
        case 5:
            stdin = freopen("./maps/map_5.txt", "r", stdin);
            break;
    }
}

void game(int mode) {
    char **matrix;
    char **buff;
    float speed = 1.0f;
    char user_button = '\0';
    alloc_memory(&matrix);
    alloc_memory(&buff);
    choose_option(mode);
    print_field(matrix);
    stdin = freopen("/dev/tty", "r", stdin);  // redirect input stream to console in Unix
    initscr();                                // initialize ncurses
    noecho();  // disable rendering of user-entered characters in terminal window
    WINDOW *win = newwin(25, 80, 0, 0);
    wrefresh(win);  // render changes made in the window
    output_field(matrix, win);
    wrefresh(win);
    while (update_field(&matrix, &buff) && !(user_button == 'q')) {
        output_field(matrix, win);
        mvwprintw(win, 24, 5, "Speed: x%.1f", 2.1 - speed);
        halfdelay(1);               // time to wait for user input
        user_button = wgetch(win);  // waits for user to press a key and returns the key code
        change_speed(user_button, &speed);
        usleep(STARTSPEED * speed);  // for a more comfortable gameplay experience (slower refresh rate)
        wrefresh(win);
    }
    free_memory(matrix);
    free_memory(buff);
    endwin();  // end work with ncurses
}

int alloc_memory(char ***matrix) {
    int check = 1;
    (*matrix) = malloc(25 * sizeof(char *));
    if (*matrix != NULL) {
        for (int i = 0; i < 25; i++) {
            (*matrix)[i] = malloc(80 * sizeof(char));
            if ((*matrix)[i] == NULL) {
                check = 0;
                for (int j = 0; j < i; j++) free((*matrix)[i]);
                free(matrix);
                break;
            }
        }
    } else {
        check = 0;
    }
    return check;
}

void free_memory(char **matrix) {
    for (int i = 0; i < 25; i++) free(matrix[i]);
    free(matrix);
}

void print_field(char **matrix) {
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            char c;
            scanf("%c ", &c);
            if (c == '-') c = '0';
            if (c == 'o') c = '1';
            matrix[i][j] = c;
        }
    }
}

int update_field(char ***matrix, char ***buff) {
    int alive_cells = 0;
    int flag = 0;
    int check = 1;
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            int count = count_alive_cells(*matrix, i, j);
            alive_cells += count;
            (*buff)[i][j] = cell_update((*matrix)[i][j], count, &flag);
        }
    }
    char **temp = *matrix;
    *matrix = *buff;
    *buff = temp;
    if (alive_cells == 0 || flag == 0) check = 0;
    return check;
}

void output_field(char **matrix, WINDOW *win) {
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            if (matrix[i][j] == '1') {
                mvwprintw(win, i, j, LIFE);
                printw(LIFE);
            } else {
                mvwprintw(win, i, j, DEAD);
            }
        }
    }
}

int count_alive_cells(char **matrix, int i, int j) {
    int count = 0;
    for (int step_i = -1; step_i <= 1; step_i++) {
        for (int step_j = -1; step_j <= 1; step_j++) {
            if ((matrix[(25 + i + step_i) % 25][(80 + j + step_j) % 80] == '1') &&
                !(step_i == 0 && step_j == 0)) {
                count++;
            }
        }
    }
    return count++;
}

char cell_update(char cell, int count, int *change_flag) {
    char new_cell;
    if (cell == '1') {
        if (count != 2 && count != 3) {
            new_cell = '0';
            *change_flag = 1;
        } else {
            new_cell = '1';
        }
    } else {
        if (count == 3) {
            new_cell = '1';
            *change_flag = 1;
        } else {
            new_cell = '0';
        }
    }
    return new_cell;
}

void change_speed(char button, float *speed) {
    if (*speed < MAXSPEED && button == 'z') {
        *speed += ITERSPEED;
    }
    if (*speed > MINSPEED && button == 'a') {
        *speed -= ITERSPEED;
    }
}