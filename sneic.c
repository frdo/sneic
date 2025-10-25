#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define GAME_BOARD_HEIGTH 20
#define GAME_BOARD_WIDTH 40
#define SNAKE_MAX_LENGHT 100

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point snake[SNAKE_MAX_LENGHT];
    int snake_lenght;
    char direction;
    Point food;
    int score;
    int game_over;
} Game;

/*
 * Generate a random int between min and max
 */
int rand_between(int min, int max) {
    return rand() % (max - min - 1) + min + 1;
}

/*
 * Initialize the game
 */
void init_game(Game* game) {
    game->snake_lenght = 1;
    game->snake[0].x = GAME_BOARD_WIDTH / 2;
    game->snake[0].y = GAME_BOARD_HEIGTH / 2;
    game->score = 0;
    game->game_over = 0;
    game->direction = 'u';
    srand(time(NULL));
    game->food.x = rand_between(0, GAME_BOARD_WIDTH);
    game->food.y = rand_between(0, GAME_BOARD_HEIGTH);
}

/*
 * Clean the terminal
 */
void clean_output() {
   printf("\033[2J\033[H");
}

/*
 * Read the user input char
 */
void get_command(char* c) {
    read(STDIN_FILENO, c, 1);
}

/*
 * Print the given char on the given point
 */
void draw_char_on_point(int x, int y, char c) {
    printf("\033[%d;%dH", y, x);
    printf("%c", c);
    fflush(stdout);
}

/*
 * Position the cursor on the given point
 */
void position_on_point(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

/*
 * Draw boundaries based on board width and heigth
 */
void draw_walls() {
    for(int y = 0; y < GAME_BOARD_HEIGTH; y++) {
        draw_char_on_point(0, y, '|');
        draw_char_on_point(GAME_BOARD_WIDTH, y, '|');
    }
    for(int x = 0;  x < GAME_BOARD_WIDTH; x++) {
        draw_char_on_point(x, 0, '-');
        draw_char_on_point(x, GAME_BOARD_HEIGTH, '-');
    }
    draw_char_on_point(0, 0, '+');
    draw_char_on_point(0, GAME_BOARD_HEIGTH, '+');
    draw_char_on_point(GAME_BOARD_WIDTH, 0, '+');
    draw_char_on_point(GAME_BOARD_WIDTH, GAME_BOARD_HEIGTH, '+');
}

/*
 * Given a position it draw a piece of snake body
 */
void draw_piece_of_snake(int x, int y) {
    position_on_point(x, y);
    printf("\033[42m \033[0m");
    fflush(stdout);
}

/*
 * Given a position it draw the food
 */
void draw_food(int x, int y) {
    position_on_point(x, y);
    printf("\033[43m \033[0m");
    fflush(stdout);
}

/*
 * Print informations, like score and how to exit the game, on the footer
 */
void draw_score(Game* game) {
    position_on_point(0, GAME_BOARD_HEIGTH+1);
    printf("Score: %d\n", game->score);
    printf("Space to exit...\n");
    if(game->game_over) {
        printf("Game Over!\n");
    }
}

/*
 * Take the user input and update the moving direction of the snake
 */
void input_game(Game* game, char* input_command) {
    get_command(input_command);
    if(*input_command != 0) {
        switch (*input_command) {
            case 'w':
                game->direction = 'u';
                break;
            case 'a':
                game->direction = 'l';
                break;
            case 's':
                game->direction = 'd';
                break;
            case 'd':
                game->direction = 'r';
                break;
        }
    }
}

void update_game(Game* game) {
    // Create a new head and position it based on direction
    Point new_head = game->snake[0];
    switch(game->direction) {
        case 'u': new_head.y--; break;
        case 'd': new_head.y++; break;
        case 'l': new_head.x--; break;
        case 'r': new_head.x++; break;
    }

    // Wall collision check
    if(new_head.x <= 1 || new_head.x >= GAME_BOARD_WIDTH || new_head.y <= 1 || new_head.y >= GAME_BOARD_HEIGTH) {
        game->game_over = 1;
        return;
    }

    // Body collision check
    for(int i = 0; i < game->snake_lenght; i++) {
        if(new_head.x == game->snake[i].x && new_head.y == game->snake[i].y) {
            game->game_over = 1;
            return;
        }
    }

    // Move the snake
    draw_char_on_point(game->snake[game->snake_lenght - 1].x, game->snake[game->snake_lenght - 1].y, ' ');
    for(int i = game->snake_lenght - 1; i > 0; i--) {
        game->snake[i] = game->snake[i-1];
        draw_piece_of_snake(game->snake[i].x, game->snake[i].y);
    }
    game->snake[0] = new_head;
    draw_piece_of_snake(game->snake[0].x, game->snake[0].y);

    // Add a new peace of body if ate food and randomly create new food
    if(new_head.x == game->food.x && new_head.y == game->food.y) {
        game->snake_lenght++;
        game->score += 1;
        game->food.x = rand_between(0, GAME_BOARD_WIDTH);
        game->food.y = rand_between(0, GAME_BOARD_HEIGTH);
        draw_food(game->food.x, game->food.y); // Draw new food
    }
}

/*
 * Setup terminal in order to have a non blocking input
 */
void setup_terminal() {
    struct termios newt;

    // Get current terminal configuration
    tcgetattr(STDIN_FILENO, &newt);

    // Disable echo and canonical mode
    newt.c_lflag &= ~(ICANON | ECHO);

    // Apply new configuration
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Set non blocking stdin
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

/*
 * Reset terminal configuration
 */
void reset_terminal() {
    struct termios oldt;

    // Reset terminal configuration
    tcgetattr(STDIN_FILENO, &oldt);
    oldt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    // Set blocking stdin
    fcntl(STDIN_FILENO, F_SETFL, 0);
}

int main() {
    printf("\033[?25l"); // Hide cursor
    Game game;
    init_game(&game);
    char input_command;

    setup_terminal();
    clean_output();
    printf("Welcome to SNEIK! Let's play!\n");
    printf("Commands:\n");
    printf("     W: GO UP\n");
    printf("     A: GO LEFT\n");
    printf("     S: GO DOWN\n");
    printf("     D: GO RIGHT\n");
    printf("\nPress any to start...\n");
    while(1) {
        get_command(&input_command);
        if(input_command > 0) {
            break;
        }
    }

    clean_output();
    draw_walls();
    draw_food(game.food.x, game.food.y);
    while(!game.game_over && input_command != ' ') {
        input_game(&game, &input_command);
        update_game(&game);
        draw_score(&game);
        usleep(500000);
    }

    reset_terminal();
    printf("\033[?25h"); // Show cursor
    return 0;
}
