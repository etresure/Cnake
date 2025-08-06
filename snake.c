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
#define STEP_PER_SEC 6
#define FPS 60

enum CellType{EMPTY = 0, FOOD, SNAKE};
enum Direction{UP, DOWN, LEFT, RIGHT};
enum State{PLAY, END, RESTART, START};
struct Snake{
    Texture2D head;
    Texture2D dead_head;
    Texture2D body_layer;
    int body[BOARD_WIDTH * BOARD_HEIGHT];
    int tail;
    int count;
    int until_step;
    enum Direction direction;
    enum Direction prev_direction;
};
Texture2D apple_pic;
enum State state;
int ham_cycle[BOARD_HEIGHT][BOARD_WIDTH] = {0};
int row_step[4] = {-1, 1, 0, 0};
int col_step[4] = {0, 0, -1, 1};
int apple;
int pause = 0;
int auto_snake = 0;
int score = 0;
int record = 0;
int flag = 0;

enum CellType board[BOARD_WIDTH * BOARD_HEIGHT] = {0};
struct Snake snake;

void draw_cell(int col, int row, enum CellType type){
    if (type == EMPTY){
        DrawRectangle(col * CELL_SIZE + 2, row * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, GetColor(0x1c1c1cff));
    }
    else if (type == FOOD){
        DrawRectangle(col * CELL_SIZE + 2, row * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, GetColor(0xd70000ff));
        DrawRectangle(col * CELL_SIZE + 20, row * CELL_SIZE + 8, 10, 10, GetColor(0xffffffff));
        DrawRectangle(col * CELL_SIZE + 8, row * CELL_SIZE + 8, 12, 24, GetColor(0xffffffff));
        DrawRectangle(col * CELL_SIZE + 10, row * CELL_SIZE + 40, 6, 6, GetColor(0xffffffff));
    }
    else if (type == SNAKE){
        DrawRectangle(col * CELL_SIZE + 2, row * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, GetColor(0x00d700ff)); 

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

void generate_ham_cycle(void){
    int n = 0;
    for (int row = 0; row < BOARD_HEIGHT; row++){
        if (row % 2 == 0)
            for (int col = 0; col < BOARD_WIDTH; col++)
                ham_cycle[row][col] = n++;
        else
            for (int col = BOARD_WIDTH - 1; col >= 0; col--)
                ham_cycle[row][col] = n++;
    }
}

void update_snake_direction_cycle(void){
    int head = (snake.tail + snake.count - 1) % (BOARD_WIDTH * BOARD_HEIGHT);
    int row = snake.body[head] / BOARD_WIDTH;
    int col = snake.body[head] % BOARD_WIDTH;
    int min_dist = BOARD_WIDTH * BOARD_HEIGHT;
    enum Direction best_dir = snake.direction;

    for (int dir = 0; dir < 4; dir++){
        int new_row = (row + row_step[dir] + BOARD_HEIGHT) % BOARD_HEIGHT;
        int new_col = (col + col_step[dir] + BOARD_WIDTH) % BOARD_WIDTH;
        int curr_val = ham_cycle[row][col];
        int neighbor_val = ham_cycle[new_row][new_col];
        int dist = (neighbor_val - curr_val + BOARD_WIDTH * BOARD_HEIGHT) % (BOARD_WIDTH * BOARD_HEIGHT);

        if (dist > 0 && dist < min_dist){
            min_dist = dist;
            best_dir = dir;
        }
    }
    snake.prev_direction = snake.direction;
    snake.direction = best_dir;
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
        if (score > record){
            record = score;
            flag = 1;
        }
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
    flag = 0;
    srand(time(NULL));
    score = 0;
    generate_snake();
    generate_apple();
    generate_ham_cycle();
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
    if (IsKeyPressed(KEY_R))
        state = RESTART;
    if (IsKeyPressed(KEY_SPACE))
        pause = !pause;
    if (IsKeyPressed(KEY_B))
        auto_snake = !auto_snake;
    if (state == START && IsKeyPressed(KEY_ENTER))
        state = PLAY;
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
        state = PLAY;
    }
}

void render_game_over(void){
    DrawText("GAME OVER!", WIDTH / 2 - 223, HEIGHT / 2 - 120, 70, GetColor(0xffffffff));
    DrawText("Press R to Restart", WIDTH / 2 - 253, HEIGHT / 2 - 50, 50, GetColor(0xffffffff));
    if (flag){
        DrawText("NEW RECORD!", WIDTH / 2 - 173, HEIGHT / 2 + 25, 50, GetColor(0xff5f00ff));
        char buff[32];
        sprintf(buff, "Record: %d", record);
        DrawText(buff, 10, 30, 20, GetColor(0xffffffff));
    }
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

void draw_body(void) {
    int end = (snake.tail + snake.count - 1) % (BOARD_WIDTH * BOARD_HEIGHT); 
    for (int i = snake.tail; i != end; i = (i + 1) % (BOARD_WIDTH * BOARD_HEIGHT)){
        int index = snake.body[i];
        int row = index / BOARD_WIDTH;
        int col = index % BOARD_WIDTH;

        DrawTextureEx(snake.body_layer, (Vector2){col * CELL_SIZE + 2, row * CELL_SIZE + 2}, 0, (CELL_SIZE - 4) / (float)snake.body_layer.width, WHITE);
    }
}

void draw_apple(void){
    int row = apple / BOARD_WIDTH;
    int col = apple % BOARD_WIDTH;
    DrawTextureEx(apple_pic, CLITERAL(Vector2){col * CELL_SIZE + 2, row * CELL_SIZE + 2}, 0, (CELL_SIZE - 4) / (float)apple_pic.width, WHITE);
}

void render(void){ 
    if (state == START){
        BeginDrawing();
        ClearBackground(GetColor(0x000000ff));
        DrawText("SNAKE GAME", WIDTH / 2 - 270, HEIGHT / 2 - 200, 80, PINK);
        DrawText("Controls:", WIDTH / 2 - 70, HEIGHT / 2 - 100, 30, RAYWHITE);
        DrawText("W A S D or Arrow Keys - Move", WIDTH / 2 - 160, HEIGHT / 2 - 60, 20, GRAY);
        DrawText("R - Restart", WIDTH / 2 - 60, HEIGHT / 2 - 30, 20, GRAY);
        DrawText("SPACE - Pause", WIDTH / 2 - 80, HEIGHT / 2, 20, GRAY);
        DrawText("B - Autopilot", WIDTH / 2 - 65, HEIGHT / 2 + 30, 20, GRAY);
        DrawText("Press ENTER to start", WIDTH / 2 - 135, HEIGHT / 2 + 80, 25, WHITE);
        DrawText("<3", WIDTH / 2 - 35, HEIGHT / 2 + 150, 80, RED);
        EndDrawing();
        return;
    }
    BeginDrawing();
    ClearBackground(GetColor(0x000000ff));
    render_board();
    draw_head();
    draw_body();
    draw_apple();
    if (state == END){
        draw_head();
        render_game_over();
    }
    char buff[32];
    sprintf(buff, "Score: %d", score);
    DrawText(buff, 10, 5, 20, GetColor(0xffffffff));
    sprintf(buff, "Record: %d", record);
    DrawText(buff, 10, 30, 20, GetColor(0xffffffff));
    if (auto_snake)
        DrawText("AUTO", 512, 5, 20, GREEN);
    if (pause)
        DrawText("PAUSE", 507, 30, 20, GREEN);   
    EndDrawing();
}

int main(void){
    InitWindow(WIDTH, HEIGHT, "snake");
    snake.head = LoadTexture("./textures/snake_head_final.png");
    if (snake.head.width == 0){
        fprintf(stderr, "Can't load the image");
        return -1;
    }
    snake.dead_head = LoadTexture("./textures/snake_head_dead.png");
    if (snake.dead_head.width == 0){
        fprintf(stderr, "Can't load the image");
        return -1;
    }
    snake.body_layer = LoadTexture("./textures/snake_body.png");
    if (snake.body_layer.width == 0){
        fprintf(stderr, "Can't load the image");
        return -1;
    }
    apple_pic= LoadTexture("./textures/apple.png");
    if (apple_pic.width == 0){
        fprintf(stderr, "Can't load the image");
        return -1;
    }
    SetTargetFPS(FPS);
    state = START;
    init();
    while (!WindowShouldClose()){
        handle_input();
        if (!pause){
            update();
        }
        if (auto_snake)
            update_snake_direction_cycle();
        render();
    }
    UnloadTexture(snake.head);
    UnloadTexture(snake.dead_head);
    UnloadTexture(apple_pic);
    CloseWindow();
    
    return 0;
}
