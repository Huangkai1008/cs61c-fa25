#include "game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/** Basic elements */
#define WALL_CHAR '#'  ///< Character representing a wall
#define EMPTY_CHAR ' ' ///< Character representing an empty cell
#define FRUIT_CHAR '*' ///< Character representing fruit

/** Snake tail characters (direction indicators) */
#define TAIL_UP 'w'    ///< Snake tail heading up
#define TAIL_LEFT 'a'  ///< Snake tail heading left
#define TAIL_DOWN 's'  ///< Snake tail heading down
#define TAIL_RIGHT 'd' ///< Snake tail heading right

/** Snake body characters (direction indicators) */
#define BODY_UP '^'    ///< Snake body heading up
#define BODY_LEFT '<'  ///< Snake body heading left
#define BODY_DOWN 'v'  ///< Snake body heading down
#define BODY_RIGHT '>' ///< Snake body heading right

/** Snake head characters (direction indicators) */
#define HEAD_UP 'W'    ///< Snake head heading up
#define HEAD_LEFT 'A'  ///< Snake head heading left
#define HEAD_DOWN 'S'  ///< Snake head heading down
#define HEAD_RIGHT 'D' ///< Snake head heading right

/** Dead snake */
#define DEAD_SNAKE 'x' ///< Snake head that has died

/**
 * @brief Direction enumeration for snake movement.
 */
typedef enum
{
    DIR_NORTH = 0, ///< Moving up
    DIR_SOUTH = 1, ///< Moving down
    DIR_EAST = 2,  ///< Moving right
    DIR_WEST = 3   ///< Moving left
} direction_t;

#define INITIAL_BUFFER_SIZE 128
#define GROWTH_FACTOR 2

/* Helper function definitions */
static void set_board_at(game_t *game, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_t *game, unsigned int snum);
static char next_square(game_t *game, unsigned int snum);
static void update_tail(game_t *game, unsigned int snum);
static void update_head(game_t *game, unsigned int snum);

/**
 * @brief Create board with given rows and columns.
 *
 * @param game The game structure.
 * @param rows The number of rows in the board.
 * @param cols The number of columns in the board.
 */
static void create_board(game_t *game, const unsigned int rows, const unsigned int cols)
{
    game->board = calloc(rows, sizeof(char *));
    if (!game->board)
    {
        fprintf(stderr, "Error: Failed to allocate board\n");
        free_game(game);
        exit(EXIT_FAILURE);
    }

    // Each row is a string, add '\n' and '\0' in end.
    for (int i = 0; i < rows; i++)
    {
        game->board[i] = calloc(cols + 2, sizeof(char));
        if (!game->board[i])
        {
            fprintf(stderr, "Error: Failed to allocate board row\n");
            free_game(game);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (i == 0 || i == rows - 1 || j == 0 || j == cols - 1)
            {
                game->board[i][j] = WALL_CHAR;
            }
            else
            {
                game->board[i][j] = EMPTY_CHAR;
            }
        }

        game->board[i][cols] = '\n';
        game->board[i][cols + 1] = '\0';
    }

    game->num_rows = rows;
}

/**
 * @brief Create default fruit.
 *
 * The fruit is at row 2, column 9 (zero-indexed).
 */
static void create_default_fruit(const game_t *game)
{
    const unsigned int row = 2;
    const unsigned int col = 9;

    game->board[row][col] = FRUIT_CHAR;
}

/**
 * @brief Create default snake.
 *
 * The tail is at row 2, column 2, and the head is at row 2, column 4.
 */
static void create_default_snake(game_t *game)
{
    game->snakes = calloc(1, sizeof(snake_t));
    if (!game->snakes)
    {
        fprintf(stderr, "Error: Failed to allocate snake");
        free_game(game);
        exit(EXIT_FAILURE);
    }

    const unsigned int tail_row = 2;
    const unsigned int tail_col = 2;
    const unsigned int head_row = 2;
    const unsigned int head_col = 4;
    game->snakes[0] =
        (snake_t){.tail_row = tail_row, .tail_col = tail_col, .head_row = head_row, .head_col = head_col, .live = true};

    game->num_snakes = 1;

    // put snake on board
    game->board[tail_row][tail_col] = TAIL_RIGHT;
    game->board[tail_row][tail_col + 1] = BODY_RIGHT;
    game->board[head_row][head_col] = HEAD_RIGHT;
}

/* Task 1 */
game_t *create_default_game()
{
    game_t *game = calloc(1, sizeof(game_t));
    if (!game)
    {
        fprintf(stderr, "Error: Failed to allocate game structure");
        exit(EXIT_FAILURE);
    }

    // The board has 18 rows, and each row has 20 columns.
    const unsigned int default_board_rows = 18;
    const unsigned int default_board_cols = 20;

    create_board(game, default_board_rows, default_board_cols);
    create_default_fruit(game);
    create_default_snake(game);
    return game;
}

/* Task 2 */
void free_game(game_t *game)
{
    if (!game)
    {
        return;
    }

    if (game->board)
    {
        for (unsigned int i = 0; i < game->num_rows; i++)
        {
            free(game->board[i]);
        }
        free(game->board);
    }

    if (game->snakes)
    {
        free(game->snakes);
    }
    free(game);
}

/* Task 3 */
// ReSharper disable once CppParameterMayBeConstPtrOrRef
void print_board(game_t *game, FILE *fp)
{
    for (int i = 0; i < game->num_rows; ++i)
    {
        fprintf(fp, "%s", game->board[i]);
    }
}

/*
  Saves the current game into filename. Does not modify the game object.
  (already implemented for you).
*/
void save_board(game_t *game, char *filename)
{
    FILE *f = fopen(filename, "w");
    print_board(game, f);
    fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_t *game, unsigned int row, unsigned int col) { return game->board[row][col]; }

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_t *game, unsigned int row, unsigned int col, char ch) { game->board[row][col] = ch; }

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) { return c == TAIL_LEFT || c == TAIL_RIGHT || c == TAIL_UP || c == TAIL_DOWN; }

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c)
{
    return c == HEAD_LEFT || c == HEAD_RIGHT || c == HEAD_UP || c == HEAD_DOWN || c == DEAD_SNAKE;
}

/*
  Returns true if c is part of the snake's body.
  The snake consists of these characters: "^<v>"
  Returns false otherwise.
*/
static bool is_body(const char c) { return c == BODY_LEFT || c == BODY_RIGHT || c == BODY_UP || c == BODY_DOWN; }

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) { return is_head(c) || is_tail(c) || is_body(c); }

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c)
{
    switch (c)
    {
    case BODY_LEFT:
        return TAIL_LEFT;
    case BODY_RIGHT:
        return TAIL_RIGHT;
    case BODY_UP:
        return TAIL_UP;
    case BODY_DOWN:
        return TAIL_DOWN;
    default:
        return c;
    }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c)
{
    switch (c)
    {
    case HEAD_LEFT:
        return BODY_LEFT;
    case HEAD_RIGHT:
        return BODY_RIGHT;
    case HEAD_UP:
        return BODY_UP;
    case HEAD_DOWN:
        return BODY_DOWN;
    default:
        return c;
    }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c)
{
    if (c == BODY_DOWN || c == HEAD_DOWN || c == TAIL_DOWN)
    {
        return cur_row + 1;
    }

    if (c == BODY_UP || c == HEAD_UP || c == TAIL_UP)
    {
        return cur_row - 1;
    }

    return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c)
{
    if (c == BODY_RIGHT || c == HEAD_RIGHT || c == TAIL_RIGHT)
    {
        return cur_col + 1;
    }

    if (c == BODY_LEFT || c == HEAD_LEFT || c == TAIL_LEFT)
    {
        return cur_col - 1;
    }

    return cur_col;
}

/*
  Task 4.2

  Helper function for update_game. Return the character in the cell the snake is
  moving into.

  This function should not modify anything.
*/
// ReSharper disable once CppParameterMayBeConst
static char next_square(game_t *game, unsigned int snum)
{
    const snake_t *snake = &game->snakes[snum];
    const char head = get_board_at(game, snake->head_row, snake->head_col);
    const unsigned int next_row = get_next_row(snake->head_row, head);
    const unsigned int next_col = get_next_col(snake->head_col, head);
    return get_board_at(game, next_row, next_col);
}

/*
  Task 4.3

  Helper function for update_game. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head.

  Note that this function ignores food, walls, and snake bodies when moving the
  head.
*/
static void update_head(game_t *game, unsigned int snum)
{
    // On the game board, add a new head where the snake is moving into,
    // and change the old head from a head character (WASD) into a body character (^).
    snake_t *snake = &game->snakes[snum];
    const char head = get_board_at(game, snake->head_row, snake->head_col);
    const unsigned int next_row = get_next_row(snake->head_row, head);
    const unsigned int next_col = get_next_col(snake->head_col, head);
    set_board_at(game, next_row, next_col, head);
    set_board_at(game, snake->head_row, snake->head_col, head_to_body(head));

    // In the snake, update the row and column of the head.
    snake->head_row = next_row;
    snake->head_col = next_col;
}

/*
  Task 4.4

  Helper function for update_game. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_t *game, unsigned int snum)
{
    // On the game board, blank out the current tail,
    // and change the new tail from a body character (^<v>) into a tail character (wasd).
    snake_t *snake = &game->snakes[snum];
    const char tail = get_board_at(game, snake->tail_row, snake->tail_col);
    const unsigned int next_row = get_next_row(snake->tail_row, tail);
    const unsigned int next_col = get_next_col(snake->tail_col, tail);
    const char body = get_board_at(game, next_row, next_col);
    set_board_at(game, snake->tail_row, snake->tail_col, EMPTY_CHAR);
    set_board_at(game, next_row, next_col, body_to_tail(body));

    // In the snake, update the row and column of the head.
    snake->tail_row = next_row;
    snake->tail_col = next_col;
}

static void update_snake_when_dead(game_t *game, const unsigned int snum)
{
    // When a snake dies, the head is replaced with an x.
    snake_t *snake = &game->snakes[snum];
    set_board_at(game, snake->head_row, snake->head_col, DEAD_SNAKE);
    snake->live = false;
}

static void update_snake_when_eat_fruit(game_t *game, const unsigned int snum)
{
    // If the head moves into a fruit,
    // the snake eats the fruit and grows by 1 unit in length.
    update_head(game, snum);
}

static void update_snake(game_t *game, const unsigned int snum)
{
    update_head(game, snum);
    update_tail(game, snum);
}

/* Task 4.5 */
void update_game(game_t *game, int (*add_food)(game_t *game))
{
    for (unsigned int snum = 0; snum < game->num_snakes; snum++)
    {
        const char square = next_square(game, snum);
        // If the head crashes into the body of a snake or a wall,
        // the snake dies and stops moving.
        if (is_snake(square) || square == WALL_CHAR)
        {
            update_snake_when_dead(game, snum);
        }

        // If the head moves into a fruit,
        // the snake eats the fruit and grows by 1 unit in length.
        // Each time fruit is consumed, a new fruit is generated on the board.
        else if (square == FRUIT_CHAR)
        {
            update_snake_when_eat_fruit(game, snum);
            add_food(game);
        }

        else
        {
            update_snake(game, snum);
        }
    }
}

/* Task 5.1 */
char *read_line(FILE *fp)
{
    // Allocate memory for the line.
    char *line = calloc(INITIAL_BUFFER_SIZE, sizeof(char));
    if (!line)
    {
        return NULL;
    }

    // Read the line from the file.
    size_t line_capacity = INITIAL_BUFFER_SIZE;
    size_t cur_pos = 0;

    while (fgets(line + cur_pos, (int) (line_capacity - cur_pos), fp))
    {
        cur_pos = strlen(line);

        // If the line contains a newline character, return the line.
        if (cur_pos > 0 && line[cur_pos - 1] == '\n')
        {
            return line;
        }

        // If the line does not contain a newline character, increase the capacity of the line.
        line_capacity *= GROWTH_FACTOR;
        char *newline = realloc(line, line_capacity);
        if (!newline)
        {
            free(line);
            return NULL;
        }

        line = newline;
    }
    free(line);
    return NULL;
}

/* Task 5.2 */
game_t *load_board(FILE *fp)
{
    game_t *game = calloc(1, sizeof(game_t));
    if (!game)
    {
        fprintf(stderr, "Error: Failed to allocate game structure");
        return NULL;
    }

    game->num_rows = 0;
    game->board = NULL;
    game->num_snakes = 0;
    game->snakes = NULL;

    for (char *line = read_line(fp); line; line = read_line(fp)) {
        char *row = malloc((strlen(line) + 1) * sizeof(char));
        if (!row)
        {
            free(line);
            free_game(game);
            return NULL;
        }

        strcpy(row, line);

        char **new_board = realloc(game->board, (game->num_rows + 1) * sizeof(char *));
        if (!new_board)
        {
            free(row);
            free(line);
            free_game(game);
            return NULL;
        }

        game->board = new_board;
        game->board[game->num_rows++] = row;
        free(line);
    }
    return game;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_t *game, unsigned int snum)
{
    snake_t *snake = &game->snakes[snum];
    unsigned int row = snake->tail_row;
    unsigned int col = snake->tail_col;
    char c = get_board_at(game, row, col);
    while (!is_head(c)) {
        row = get_next_row(row, c);
        col = get_next_col(col, c);
        c = get_board_at(game, row, col);
    }

    snake->head_row = row;
    snake->head_col = col;
}

/* Task 6.2 */
game_t *initialize_snakes(game_t *game)
{
    unsigned int num_snakes = 0;
    snake_t *snakes = NULL;
    for (unsigned int row = 0; row < game->num_rows; row++)
    {
        for (unsigned int col = 0; col < strlen(game->board[row]); col++)
        {
            const char c = get_board_at(game, row, col);
            if (is_tail(c))
            {
                snake_t *original_snakes = snakes;
                snakes = realloc(snakes, (num_snakes + 1) * sizeof(snake_t));
                if (!snakes)
                {
                    free(original_snakes);
                    return NULL;
                }

                snake_t *snake = &snakes[num_snakes];
                snake->tail_row = row;
                snake->tail_col = col;
                game->snakes = snakes;

                find_head(game, num_snakes);
                const char head = get_board_at(game, snake->head_row, snake->head_col);
                snake->live = head != DEAD_SNAKE;
                num_snakes++;
            }
        }
    }
    game->num_snakes = num_snakes;
    return game;
}
