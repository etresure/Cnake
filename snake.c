#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "raylib.h"

#define WIDTH 1080
#define HEIGHT 720
#define CELL_SIZE 60
#define BOARD_WIDTH (WIDTH / CELL_SIZE)
#define BOARD_HEIGHT (HEIGHT / CELL_SIZE)
#define STEP_PER_SEC 60
#define FPS 60

enum CellType{EMPTY = 0, FOOD, SNAKE};
enum Direction{UP, DOWN, LEFT, RIGHT};
enum State{PLAY, END, RESTART};
struct Snake{
    Texture2D head;
    Texture2D dead_head;
    int body[BOARD_WIDTH * BOARD_HEIGHT];
    int tail;
    int count;
    int until_step;
    enum Direction direction;
    enum Direction prev_direction;
};
enum State state;
int row_step[4] = {-1, 1, 0, 0};
int col_step[4] = {0, 0, -1, 1};
int apple;
int pause = 0;
int score = 0;

enum CellType board[BOARD_WIDTH * BOARD_HEIGHT] = {0};
struct Snake snake;

void draw_cell(int col, int row, enum CellType type){
    if (type == EMPTY){
        DrawRectangle(col * CELL_SIZE + 2, row * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, GetColor(0x333333ff));
    }
    else if (type == FOOD){
        DrawRectangle(col * CELL_SIZE + 2, row * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, GetColor(0xff5555ff));
    }
    else if (type == SNAKE){
        DrawRectangle(col * CELL_SIZE + 2, row * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, GetColor(0x55ff55ff));
    }
    else{
        assert(0 && "unreachable");
    }
}

void render_board(void){
    for (int col = 0; col < BOARD_WIDTH; col++){
        for (int row = 0; row < BOARD_HEIGHT; row++){
            draw_cell(col, row, board[row * BOARD_WIDTH + col]);
        }
    }
}

void generate_snake(void){
    snake.tail = 0;
    snake.count = 1;
    snake.body[snake.tail] = (BOARD_HEIGHT / 2) * BOARD_WIDTH + BOARD_WIDTH / 2;
    board[snake.body[snake.tail]] = SNAKE;
    snake.direction = UP;
    snake.prev_direction = -1;
    snake.until_step = 0;
}

int check_apple(int row, int col){
    int i = row * BOARD_WIDTH + col;
    return apple == i;
}

int check_collision(int row, int col){
    int i = row * BOARD_WIDTH + col;
    int end = (snake.tail + snake.count) % (BOARD_WIDTH * BOARD_HEIGHT);
    for (int j = snake.tail; j != end; j = (j + 1) % (BOARD_WIDTH * BOARD_HEIGHT)){
        if (i == snake.body[j])
            return 1;
    }
    return 0;
}

void generate_apple(void){
    int max = BOARD_WIDTH * BOARD_HEIGHT - 1;
    int min = 0;
    int r = min + rand() % (max - min + 1);
    while (board[r] != EMPTY){
        r = min + rand() % (max - min + 1);
    }
    board[r] = FOOD;
    apple = r;
}

void snake_step(void){
    int head = (snake.tail + snake.count - 1) % (BOARD_WIDTH * BOARD_HEIGHT);
    int row = snake.body[head] / BOARD_WIDTH;
    int col = snake.body[head] % BOARD_WIDTH;
    row = (row + row_step[snake.direction] + BOARD_HEIGHT) % BOARD_HEIGHT;
    col = (col + col_step[snake.direction] + BOARD_WIDTH) % BOARD_WIDTH;
    snake.prev_direction = snake.direction;
    if (check_apple(row, col)){
        int new_head = row * BOARD_WIDTH + col;
        snake.count++;
        snake.body[(snake.tail + snake.count - 1) % (BOARD_WIDTH * BOARD_HEIGHT)] = new_head;
        board[new_head] = SNAKE;
        assert(new_head == apple);
        generate_apple();
        score++;
    }
    else if (check_collision(row, col)){
        state = END;
    }
    else{
        board[snake.body[snake.tail]] = EMPTY;
        snake.tail = (snake.tail + 1) % (BOARD_WIDTH * BOARD_HEIGHT);
        int new_head = row * BOARD_WIDTH + col;
        snake.body[(snake.tail + snake.count - 1) % (BOARD_WIDTH * BOARD_HEIGHT)] = new_head;
        board[new_head] = SNAKE;
        
    }
}

void init(void){
    srand(time(NULL));
    state = PLAY;
    score = 0;
    generate_snake();
    generate_apple();
}

void handle_input(void){
    if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_S)) && snake.prev_direction != DOWN && pause == 0)
        snake.direction = UP;
    if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_W)) && snake.prev_direction != UP && pause == 0) 
        snake.direction = DOWN;
    if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_D)) && snake.prev_direction != RIGHT && pause == 0) 
        snake.direction = LEFT;
    if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_A)) && snake.prev_direction != LEFT && pause == 0) 
        snake.direction = RIGHT;    
    if ((IsKeyPressed(KEY_R))){
        state = RESTART;
    }
    if ((IsKeyPressed(KEY_SPACE)))
        pause = !pause;
}

void update(void){
    if (state == PLAY){
        if (snake.until_step == 0){
            snake_step();
            snake.until_step = FPS / STEP_PER_SEC;
        }
        snake.until_step--;
    }
    else if (state == RESTART){
        memset(board, 0, BOARD_HEIGHT * BOARD_WIDTH * sizeof(board[0]));
        init();
    }
}

void render_game_over(void){
    DrawText("GAME OVER", WIDTH / 2 - 150, HEIGHT / 2 - 25, 50, GetColor(0xffffffff));
    DrawText("Press R to Restart", WIDTH / 2 - 250, HEIGHT / 2 + 25, 50, GetColor(0xffffffff));
}

void draw_head(void){
    int head = (snake.tail + snake.count - 1) % (BOARD_WIDTH * BOARD_HEIGHT);
    int row = snake.body[head] / BOARD_WIDTH;
    int col = snake.body[head] % BOARD_WIDTH;
    if (snake.direction == RIGHT){
        if (state == END)
            DrawTextureEx(snake.dead_head, CLITERAL(Vector2){col * CELL_SIZE + 2 + CELL_SIZE - 4, row * CELL_SIZE + 2}, 90, (CELL_SIZE - 4) / (float)snake.dead_head.width, WHITE);
        else
            DrawTextureEx(snake.head, CLITERAL(Vector2){col * CELL_SIZE + 2 + CELL_SIZE - 4, row * CELL_SIZE + 2}, 90, (CELL_SIZE - 4) / (float)snake.head.width, WHITE);
    }
    else if (snake.direction == UP){
        if (state == END)
            DrawTextureEx(snake.dead_head, CLITERAL(Vector2){col * CELL_SIZE + 2, row * CELL_SIZE + 2}, 0, (CELL_SIZE - 4) / (float)snake.dead_head.width, WHITE);
        else
            DrawTextureEx(snake.head, CLITERAL(Vector2){col * CELL_SIZE + 2, row * CELL_SIZE + 2}, 0, (CELL_SIZE - 4) / (float)snake.head.width, WHITE);
    }
    else if (snake.direction == LEFT){
        if (state == END)
            DrawTextureEx(snake.dead_head, CLITERAL(Vector2){col * CELL_SIZE + 2, row * CELL_SIZE + 2 + CELL_SIZE - 4}, -90, (CELL_SIZE - 4) / (float)snake.dead_head.width, WHITE);
        else
            DrawTextureEx(snake.head, CLITERAL(Vector2){col * CELL_SIZE + 2, row * CELL_SIZE + 2 + CELL_SIZE - 4}, -90, (CELL_SIZE - 4) / (float)snake.head.width, WHITE);
    }
    else if (snake.direction == DOWN){
        if (state == END)
            DrawTextureEx(snake.dead_head, CLITERAL(Vector2){col * CELL_SIZE + 2 + CELL_SIZE - 4, row * CELL_SIZE + 2 + CELL_SIZE - 4}, 180, (CELL_SIZE - 4) / (float)snake.dead_head.width, WHITE);
        else
            DrawTextureEx(snake.head, CLITERAL(Vector2){col * CELL_SIZE + 2 + CELL_SIZE - 4, row * CELL_SIZE + 2 + CELL_SIZE - 4}, 180, (CELL_SIZE - 4) / (float)snake.head.width, WHITE);
    }
}

void render(void){ 
    BeginDrawing();
    ClearBackground(GetColor(0x000000ff));
    render_board();
    draw_head();
    if (state == END){
        render_game_over();
        draw_head();
    }
    char buff[32];
    sprintf(buff, "Score: %d", score);
    DrawText(buff, 10, 10, 20, GetColor(0xffffffff));
    EndDrawing();
}

int main(void){
    InitWindow(WIDTH, HEIGHT, "snake");
    snake.head = LoadTexture("./snake_head_final.png");
    if (snake.head.width == 0){
        fprintf(stderr, "Can't load the image");
        return -1;
    }
    snake.dead_head = LoadTexture("./snake_head_dead.png");
    if (snake.dead_head.width == 0){
        fprintf(stderr, "Can't load the image");
        return -1;
    }
    SetTargetFPS(FPS);
    init();
    while (!WindowShouldClose()){
        handle_input();
        if (!pause){
            update();
        }
        render();
    }
    UnloadTexture(snake.head);
    UnloadTexture(snake.dead_head);
    CloseWindow();
    
    return 0;
}



//gcc 2.c -l raylib -L . -l glfw3 -framework Cocoa -framework IOKit -g3
//kitty/launcher/kitty