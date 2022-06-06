#ifndef SNAKE_H
#define SNAKE_H

#include <SDL2/SDL.h>

#define BOARD_WIDTH 20
#define BOARD_HEIGHT 20
#define TILE_SIZE 30

#define START_X BOARD_WIDTH / 2
#define START_Y BOARD_HEIGHT / 2
#define START_LENGTH 4

typedef struct Point
{
    int x;
    int y;
} Point;

enum Direction
{
    RIGHT,
    UP,
    LEFT,
    DOWN
};

struct SnakeSegment
{
    Point pos;
    enum Direction from;
    enum Direction to;
    struct SnakeSegment *next;
    struct SnakeSegment *prev;
};

enum State
{
    RUNNING,
    GAME_OVER
};

enum Textures
{
    HEAD,
    STRAIGHT,
    TURN,
    TAIL,
    FOOD
};

void init(void);
void gameInit(void);
void gameClose(void);
void close(void);
void handleEvent(SDL_Event e);
void spawnFood(void);
void moveSnake(void);
void drawSnake(void);
void drawScreen(void);
Point boardToScreen(Point b);

#endif 