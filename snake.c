#include "snake.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Texture *spritesheet = NULL;
TTF_Font *font = NULL;

SDL_Rect texRects[5];

int board[BOARD_WIDTH][BOARD_HEIGHT];
struct SnakeSegment *head = NULL;
struct SnakeSegment *tail = NULL;
enum Direction facing = RIGHT;
Point foodPos = (Point){-1, -1};

enum State state = RUNNING;

int hasMoved = 0;

int score = 0;

int quit = 0;

int main(int argc, char *argv[])
{
    init();
    gameInit();

    SDL_Event e;

    while (!quit)
    {
        hasMoved = 0;
        while (SDL_PollEvent(&e))
        {
            handleEvent(e);
        }
        if (state == RUNNING)
        {
            moveSnake();
            spawnFood();
        }

        drawScreen();

        SDL_Delay(125);
    }

    gameClose();
    close();

    return 0;
}

void init()
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, BOARD_WIDTH * TILE_SIZE,
                              BOARD_HEIGHT * TILE_SIZE, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    spritesheet = IMG_LoadTexture(renderer, "snake.png");
    texRects[HEAD] = (SDL_Rect){.x = 0, .y = 0, .w = 100, .h = 100};
    texRects[STRAIGHT] = (SDL_Rect){.x = 100, .y = 0, .w = 100, .h = 100};
    texRects[TURN] = (SDL_Rect){.x = 200, .y = 0, .w = 100, .h = 100};
    texRects[TAIL] = (SDL_Rect){.x = 300, .y = 0, .w = 100, .h = 100};
    texRects[FOOD] = (SDL_Rect){.x = 400, .y = 0, .w = 100, .h = 100};

    font = TTF_OpenFont("font.ttf", 50);
}

void gameInit()
{
    facing = RIGHT;
    score = 0;
    state = RUNNING;

    for (int i = 0; i < BOARD_WIDTH; i++)
    {
        for (int j = 0; j < BOARD_HEIGHT; j++)
        {
            board[i][j] = 0;
        }
    }

    head = malloc(sizeof(*head));
    *head = (struct SnakeSegment){(Point){START_X, START_Y}, LEFT, 0, NULL, NULL};

    tail = head;

    struct SnakeSegment *cur;
    for (int i = 1; i < START_LENGTH; i++)
    {
        cur = malloc(sizeof(*cur));
        tail->next = cur;
        *cur = (struct SnakeSegment){(Point){START_X - i, START_Y}, LEFT, RIGHT, NULL, tail};
        tail = cur;
        board[START_X - i][START_Y] = 1;
    }

    board[START_X][START_Y] = 1;

    foodPos = (Point){-1, -1};

    srand(time(NULL));
}

void gameClose()
{
    struct SnakeSegment *cur = head;
    while (cur != NULL)
    {
        struct SnakeSegment *temp = cur->next;
        free(cur);
        cur = temp;
    }
    head = NULL;
    tail = NULL;
}

void close()
{
    SDL_DestroyWindow(window);
    window = NULL;
    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    SDL_DestroyTexture(spritesheet);

    TTF_CloseFont(font);
    font = NULL;

    SDL_Quit();
    IMG_Quit();
    TTF_Quit();
}

void handleEvent(SDL_Event e)
{
    if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
    {
        quit = 1;
    }
    if (e.type == SDL_KEYDOWN)
    {
        switch (state)
        {
        case RUNNING:
            if (!hasMoved)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_UP:
                    if (facing != DOWN)
                        facing = UP;
                    break;
                case SDLK_DOWN:
                    if (facing != UP)
                        facing = DOWN;
                    break;
                case SDLK_LEFT:
                    if (facing != RIGHT)
                        facing = LEFT;
                    break;
                case SDLK_RIGHT:
                    if (facing != LEFT)
                        facing = RIGHT;
                    break;
                }
                hasMoved = 1;
            }
            break;
        case GAME_OVER:
            if (e.key.keysym.sym == SDLK_SPACE)
            {
                gameClose();
                gameInit();
            }
        }
    }
}

int randint(int max)
{
    return (int)((float)rand() / RAND_MAX * max);
}

void spawnFood()
{
    if (foodPos.x < 0 && foodPos.y < 0)
        while (board[foodPos.x = randint(BOARD_WIDTH)]
                    [foodPos.y = randint(BOARD_HEIGHT)])
            ;
}

void moveSnake()
{
    struct SnakeSegment *newSeg = malloc(sizeof(*newSeg));
    *newSeg = (struct SnakeSegment){.next = head, .prev = NULL};
    head->prev = newSeg;

    head->to = facing;
    newSeg->from = (facing + 2) % 4;

    switch (facing)
    {
    case LEFT:
        newSeg->pos = (Point){(head->pos.x - 1) % BOARD_WIDTH, head->pos.y};
        if (newSeg->pos.x < 0)
            newSeg->pos.x += 20;
        break;
    case RIGHT:
        newSeg->pos = (Point){(head->pos.x + 1) % BOARD_WIDTH, head->pos.y};
        break;
    case UP:
        newSeg->pos = (Point){head->pos.x, (head->pos.y - 1) % BOARD_HEIGHT};
        if (newSeg->pos.y < 0)
            newSeg->pos.y += 20;
        break;
    case DOWN:
        newSeg->pos = (Point){
            head->pos.x, (head->pos.y + 1) % BOARD_HEIGHT};
        break;
    }
    head = newSeg;

    if (head->pos.x == foodPos.x && head->pos.y == foodPos.y)
    {
        foodPos = (Point){-1, -1};
        score++;
    }
    else
    {
        board[tail->pos.x][tail->pos.y] = 0;
        struct SnakeSegment *temp = tail->prev;
        temp->next = NULL;
        free(tail);
        tail = temp;
    }
    if (board[head->pos.x][head->pos.y])
    {
        state = GAME_OVER;
        printf("%d\n", score);
    }
    else
    {
        board[head->pos.x][head->pos.y] = 1;
    }
}

void drawSnake()
{
    struct SnakeSegment *curSeg = head->next;
    SDL_Rect r = {.w = TILE_SIZE, .h = TILE_SIZE};
    int rot = 0;
    SDL_Rect *clip;
    while (curSeg != NULL)
    {
        r.x = boardToScreen(curSeg->pos).x;
        r.y = boardToScreen(curSeg->pos).y;

        if (curSeg == tail)
        {
            clip = &texRects[TAIL];
            switch (curSeg->to)
            {
            case RIGHT:
                rot = 180;
                break;
            case DOWN:
                rot = 270;
                break;
            case LEFT:
                rot = 0;
                break;
            case UP:
                rot = 90;
                break;
            }
        }
        else if (curSeg->from % 2 == curSeg->to % 2)
        {
            clip = &texRects[STRAIGHT];
            if (curSeg->from == LEFT || curSeg->to == LEFT)
            {
                rot = 0;
            }
            else
            {
                rot = 90;
            }
        }
        else
        {
            clip = &texRects[TURN];

            if (curSeg->from == LEFT && curSeg->to == DOWN || curSeg->from == DOWN && curSeg->to == LEFT)
            {
                rot = 0;
            }
            else if (curSeg->from == LEFT && curSeg->to == UP || curSeg->from == UP && curSeg->to == LEFT)
            {
                rot = 90;
            }
            else if (curSeg->from == RIGHT && curSeg->to == UP || curSeg->from == UP && curSeg->to == RIGHT)
            {
                rot = 180;
            }
            else if (curSeg->from == RIGHT && curSeg->to == DOWN || curSeg->from == DOWN && curSeg->to == RIGHT)
            {
                rot = 270;
            }

            else
            {
                rot = 0;
                clip = &texRects[0];
            }
        }

        SDL_RenderCopyEx(renderer, spritesheet, clip, &r, rot, NULL, 0);
        
        curSeg = curSeg->next;
    }

    r.x = boardToScreen(head->pos).x;
    r.y = boardToScreen(head->pos).y;

    clip = &texRects[HEAD];
    switch (head->from)
    {
    case RIGHT:
        rot = 0;
        break;
    case DOWN:
        rot = 90;
        break;
    case LEFT:
        rot = 180;
        break;
    case UP:
        rot = 270;
        break;
    }
    SDL_RenderCopyEx(renderer, spritesheet, clip, &r, rot, NULL, 0);
}

SDL_Texture *textTexture(char* text, TTF_Font *font, SDL_Color color){
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void renderText(char* text, TTF_Font *font, SDL_Color color, SDL_Rect *dst){
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, dst);
    SDL_DestroyTexture(texture);
}

void drawScreen()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    drawSnake();
    if (foodPos.x >= 0 && foodPos.y >= 0)
    {
        SDL_Rect food = {.w = TILE_SIZE, .h = TILE_SIZE, .x = boardToScreen(foodPos).x, .y = boardToScreen(foodPos).y};
        SDL_RenderCopy(renderer, spritesheet, &texRects[FOOD], &food);
    }
    SDL_Rect textRect = {10, 10, 30, 30};
    char *scoreStr = malloc(4 * sizeof(char));
    sprintf(scoreStr, "%d", score);
    renderText(scoreStr, font, (SDL_Color){255, 255, 255}, &textRect);

    if(state == GAME_OVER){
        SDL_Rect gameOverRect = {100, 200, 400, 100};
        renderText("GAME OVER", font, (SDL_Color){255, 255, 255}, &gameOverRect);
    }


    SDL_RenderPresent(renderer);
}

Point boardToScreen(Point b)
{
    return (Point){b.x * TILE_SIZE, b.y * TILE_SIZE};
}
