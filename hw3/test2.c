#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <curses.h>

#define NUM_mines 10
#define HEIGHT 10
#define WIDTH 10

bool lose = false;
bool win = false;
int mines = NUM_mines;
int Flag = NUM_mines;


typedef struct Cell {
    int number_of_mines;
    char ch;
    bool uncovered;
    bool bomb;
} Cell;

/* struct members for height and width of board and 2 dimensional array of cells */
typedef struct Board {
    int height, width;
    Cell **cells; /* pointer to the pointer */
} Board;

void make_board(Board *);          /* pointer to the board in order to create board */
void print_board(Board *);         /* pointer to the board and printing board */
void bombplacing_randomly(Board *, int);    /* pointer to the board and number of bombs  */
void numof_adjacent_mines(Board *);        /* pointer to the board and determining number of bombs in adjacency cells */
void uncover(Board *, int, int);    /* pointer to the board and selected coordinates by the user for uncovering cells */
void reveal_automatically(Board *, int, int);   /* pointer to the board and cell's coordinates for automatic reveal of cells */
void check_for_win(Board *, int);   /* pointer to the board and number of mines  */
void play_game();                   /* function to play the game */

int main() {
    clock_t begin = clock(); /* getting time at the beginning */

    initscr();  // Initialize the screen
    cbreak();   // Disable line buffering

    play_game();

    endwin();   // End curses mode

    clock_t end = clock();  /* getting time at the end */
    double spend_time = (double)(end - begin) / CLOCKS_PER_SEC; /* time difference */

    printf("You spent %.2f seconds playing the game\n", spend_time);
    return 0;
}

void make_board(Board *ptr) {
    int i, j;
    ptr->cells = (Cell **)malloc((ptr->height + 2) * sizeof(Cell *)); /* array of pointers*/

    for (i = 0; i <= ptr->height + 1; ++i)
        ptr->cells[i] = (Cell *)malloc((ptr->width + 2) * sizeof(Cell)); /* dynamic 2 dimensional array of cells and creating cells in board */
    for (i = 0; i <= ptr->height + 1; ++i) {
        for (j = 0; j <= ptr->width + 1; ++j) {
            if (i == 0 || i == ptr->height + 1 || j == 0 || j == ptr->width + 1) {
                ptr->cells[i][j].bomb = false;
                ptr->cells[i][j].uncovered = true;
            }

            else {
                ptr->cells[i][j].ch = ' ';  // making cells with character ' '
                ptr->cells[i][j].number_of_mines = 0; // before the game it is zero
                ptr->cells[i][j].bomb = false; // there is no bomb before the game
                ptr->cells[i][j].uncovered = false;
            }
        }
    }
    return;
}

void print_board(Board *ptr) {
    clear(); // Clear the screen

    printw("a\na\na\n");
    printw("FLAGS: %d\n", Flag);
    printw("  ");
    int i, j;
    for (i = 0; i <= ptr->width-1; ++i) /* loop in order print each row */
        printw("%d ", i); // numbers which shows rows
    printw("\n");
    /* nested loop in order to print numbers for columns and characters for cells */
    for (i = 0; i <= ptr->height-1; ++i) {
        for (j = 0; j <= ptr->width; ++j) {
            if (j == 0){
                printw("%d ", i);
            } 
            else{
                if(ptr->cells[i+1][j].ch == '0'){
                    printw("X ");
                } else{
                    printw("%c ", ptr->cells[i+1][j].ch);
                }
            }
                 // printing cells in the board
        }
        printw("\n");
    }
    refresh(); // Refresh the screen
    return;
}

void bombplacing_randomly(Board *ptr, int mines) {
    int random_row, random_col, num_of_mine = 0;
    while (num_of_mine < mines) {
        random_row = rand() % ptr->height; // generating random number
        random_col = rand() % ptr->width;  // generating random number

        // if (ptr->cells[random_row][random_col].bomb == false && (random_row != 0 && random_col != 0)) // checking for numbers which were generated before or not
        if (ptr->cells[random_row][random_col].bomb == false)
        {
            ptr->cells[random_row][random_col].bomb = true; // if not, make a new bomb
            num_of_mine++;
        }
    }
    return;
}

void numof_adjacent_mines(Board *ptr) {
    /*
	Count all the mines in the 8 adjacent
        cells

      (i-1,j-1)  (i-1,j) (i-1,j+1)
              \    |    /
               \   |   /
        (i,j-1)---CELL---(i,j+1)
                 / |  \
               /   |    \
       (i+1,j-1) (i+1,j) (i+1,j+1)
    */

    int i, j, m, n;
    for (i = 1; i <= ptr->height; ++i) {
        for (j = 1; j <= ptr->width; ++j) {
            if (ptr->cells[i][j].bomb == false) {
                for (m = i - 1; m <= i + 1; ++m)
                    for (n = j - 1; n <= j + 1; ++n)
                        if (ptr->cells[m][n].bomb == true) // checking number of bombs in adjacent cell
                            ptr->cells[i][j].number_of_mines++;
            }
        }
    }
    return;
}

void uncover(Board *ptr, int a, int b) {
    if (ptr->cells[a][b].bomb == true) {
        lose = true; // terminate the game "in the play game function (while loop)"
        int i, j;
        /* nested loop to uncover cells */
        for (i = 1; i <= ptr->height; ++i)
            for (j = 1; j <= ptr->width; ++j)
                if (ptr->cells[i][j].bomb == true) /* if there is a bomb */
                    ptr->cells[i][j].ch = '*'; // showing all bombs
                else
                    ptr->cells[i][j].ch = ptr->cells[i][j].number_of_mines + '0'; // putting number of mines in surrounding cells
        print_board(ptr);
        printw("\nYou Lost\n");
        printw("Game Over\n");
        refresh(); // Refresh the screen

        return;
    }

    ptr->cells[a][b].ch = ptr->cells[a][b].number_of_mines + '0'; //  showing number of bombs in the cell
    if (ptr->cells[a][b].number_of_mines == 0)
        reveal_automatically(ptr, a, b); // if number of bomb is 0 in cell then reveal automatically
    else
        ptr->cells[a][b].uncovered = true;

    return;
}

void reveal_automatically(Board *ptr, int a, int b) {
    int i, j;
    if (ptr->cells[a][b].uncovered == false) {
        ptr->cells[a][b].uncovered = true;
        /* nested loop to reveal automatically */
        for (i = a - 1; i <= a + 1; ++i)
            for (j = b - 1; j <= b + 1; ++j)
                if (ptr->cells[i][j].uncovered == false)
                    uncover(ptr, i, j); //calling function in order to uncover cells
    }
    return;
}

void check_for_win(Board *ptr, int mines) {
    int i, j, counter = 0; /* number of cells without bombs. At first assigning to zero*/

    for (i = 1; i <= ptr->height; ++i)
        for (j = 1; j <= ptr->width; ++j)
            if (ptr->cells[i][j].bomb == false && ptr->cells[i][j].ch != ' ' && ptr->cells[i][j].ch != 'F') /*if no bomb. increment*/
                counter++; /* incrementing cells */

    /* if counter equal below equation, it means user found all the cells which does not contain mines*/
    if (counter == (ptr->height * ptr->width) - mines) {
        win = true;
        for (i = 1; i <= ptr->height; ++i)
            for (j = 1; j <= ptr->width; ++j)
                if (ptr->cells[i][j].bomb == true) /* if there is a bomb */
                    ptr->cells[i][j].ch = '*'; // show all the bombs
                else
                    ptr->cells[i][j].ch = ptr->cells[i][j].number_of_mines + '0'; // showing numbers in the cells

        print_board(ptr);
        printw("\nYou Won, Congratulations!!!\n");
        refresh(); // Refresh the screen

        return;
    }
}

void play_game() {

    srand(time(NULL)); // set seed for rand() in the "bombplacing_randomly" function

    // printw("\t\t\t\t ***WELCOME TO MINEWSWEEPER GAME*** \n");
    int x, y, row, column;
    char op;

    // printw("Please enter the size of board:\n");
    // scanw("%d%d", &x, &y);

    // printw("Please enter the number of mines: ");
    // scanw("%d", &mines);

    Board *ptr = (Board *)malloc(sizeof(Board)); /* creating dynamic array */
    ptr->height = HEIGHT;
    ptr->width = WIDTH;
  
    make_board(ptr);
    bombplacing_randomly(ptr, mines);
    numof_adjacent_mines(ptr);

    do {
        print_board(ptr);

        // printw("Would you like to uncover or mark?  'u' or 'm' ");
        // op = getch(); // Get a character from user
        // refresh(); // Refresh the screen

        printw("X: ");
        scanw("%d", &row);
        printw("Y: ");
        scanw("%d", &column);
        printw("A: ");
        scanw("%c", &op);

        row++;
        column++;
        if (op == 'c') /* u -> uncover */
            uncover(ptr, row, column);
        if (op == 'f'){ /* m -> mark*/
            ptr->cells[row][column].ch = 'F';
            Flag--;
        }
        if (op == 'x'){
            ptr->cells[row][column].ch = ' ';
            Flag++;
        }
        if (!lose)
            check_for_win(ptr, mines);
    } while (!lose && !win);

    /* memory releasing */
    for (int i = 0; i <= ptr->height + 1; ++i)
        free(ptr->cells[i]); /* free each cell */
    free(ptr->cells); /* free array of pointers */
    return;
}
