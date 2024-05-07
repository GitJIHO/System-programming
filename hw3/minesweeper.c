#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

//magic numbers
#define X_SIZE 10
#define Y_SIZE 10
#define NUM_MINES 15
#define empty 0
#define mine 1
#define noView 0
#define view 1
#define noFlag 0
#define flag 1

//point
typedef struct {
    int x;
    int y;
} point;

//forward declarations
point* getRandomPoint();
void populateFields();
void debugPrint();
void reDrawPlayerView();
int getMineNum(int x, int y);
int isMine(int x, int y);
int dig(int x, int y);
void revealFrom(int x, int y);
int isWin();
int getPlayerInput();
int setFlag(int x, int y);
int checkMine(int x, int y);
int landingMenu();

//this is the main array used for game logic
int minefield[X_SIZE][Y_SIZE];
//array that keeps track of what the player can see
int visibleField[X_SIZE][Y_SIZE];
//array that keeps track of the player's flags
int flagField[X_SIZE][Y_SIZE];
//set to true if the player loses
int lose = 0;

int main (int argc, char ** argv)
{
    if (!landingMenu())
        return 1;

    srand(time(NULL)); //seed the random number generator
    populateFields(); //set up the game world
    reDrawPlayerView(); //draw the game world

    while (!isWin())
    {
        if (!getPlayerInput())
            system("sleep 1");
        if (lose)
        {
            printf("\x1B[32m" "" "\x1B[0m"); //reset the printf color, idk if I actually need to do this, but I will just in case
            return 1;
        }

        reDrawPlayerView();
    }

    printf("YOU WIN!\n");

    printf("\x1B[32m" "" "\x1B[0m"); //reset the printf color, idk if I actually need to do this, but I will just in case
    return 1;
}

int landingMenu()
{
    char in;
    system("clear");

    while (1)
    {
        printf("Welcome to minesweeper!\n1- Play\n2- Quit\n");

        //fseek(stdin,0,SEEK_END); //uncomment for solaris system
        scanf("%1s", &in);
        if (in == '1')
        {
            return 1;
        } else if (in == '2')
        {
            return 0;
        } else {
            printf("Invalid input!\n");
            system("sleep 1");
            system("clear");
        }
    }
}

//returns a pointer to a random point
point* getRandomPoint()
{
    point * p = (point*)(malloc(sizeof(point))); //allocate memory for the point

    p->x = (rand() % X_SIZE);
    p->y = (rand() % Y_SIZE);

    return p;
}

//sets up the game
void populateFields()
{
    int c = 0;
    int i, j;
    point *p;

    //initialize the minefield
    for (i = 0; i < Y_SIZE; i++)
        for (j = 0; j < X_SIZE; j++)
            minefield[j][i] = empty;

    //initialize the playerfield
    for (i = 0; i < Y_SIZE; i++)
        for (j = 0; j < X_SIZE; j++)
            visibleField[j][i] = noView;

    //initialize the flagfield
    for (i = 0; i < Y_SIZE; i++)
        for (j = 0; j < X_SIZE; j++)
            flagField[j][i] = noFlag;

    while (1)
    {
        p = getRandomPoint();

        if (minefield[p->x][p->y] != mine)
        {
            minefield[p->x][p->y] = mine;
            c++;
        }
        free(p); //free the memory allocated in getRandomPoint

        if (c >= NUM_MINES)
            break;
    }
}

//prints a view of the minefield for debugging
void debugPrint()
{
    int i, j;
    printf("\x1B[32m" "-------------------------------------------\n");

    for (i = 0; i < Y_SIZE; i++)
    {
        printf("\x1B[32m" "| ");
        for (j = 0; j < X_SIZE; j++)
        {
            if (minefield[j][i] == mine)
                printf("\x1B[31m" "X | ");
            else
            {
                printf("\x1B[36m" "%d | ", getMineNum(j, i));
            }
        }
        printf("\x1B[32m" "\n");
        printf("\x1B[32m" "-------------------------------------------\n");
    }
}

//draws or redraws the player's view
void reDrawPlayerView()
{
    int i, j;

    system("clear");

    printf("\x1B[32m" "----1---2---3---4---5---6---7---8---9---10-\n");

    for (i = 0; i < Y_SIZE; i++)
    {
        if (i+1 < 10)
            printf("\x1B[32m" "%d | ", i+1);
        else
            printf("\x1B[32m" "%d| ", i+1);
        for (j = 0; j < X_SIZE; j++)
        {
            if (flagField[j][i] == flag)
            {
                printf("\x1B[31m"  "F | ");
            }
            else if (visibleField[j][i] == noView)
                printf("\x1B[32m"  "# | ");
            else
            {
                printf("\x1B[36m" "%d | ", getMineNum(j, i));
            }
        }
        printf("\x1B[32m" "\n");
        printf("\x1B[32m" "-------------------------------------------\n");
    }
}

//'digs' at the current point
int dig(int x, int y)
{
    if (visibleField[x][y] == view)
    {
        printf("Already dug here.\n");
        system("sleep 1");
        return 1;
    }
    if (flagField[x][y] == flag)
    {
        printf("Remove flag before digging!\n");
        system("sleep 1");
        return 1;
    }
    if (minefield[x][y] == mine)
    {
        debugPrint();
        printf("YOU LOSE!\n");
        lose = 1;
        return 1;
    }

    revealFrom(x, y);

    return 1;
}

void revealFrom(int x, int y)
{
    if (x >= X_SIZE || x < 0 || y >= Y_SIZE || y < 0)
    {
        return;
    }
    else if (visibleField[x][y] == view)
    {
        return;
    }
    else if (minefield[x][y] == mine)
    {
        return;
    }
    else if (getMineNum(x, y) > 0)
    {
        visibleField[x][y] = view;
        return;
    } else {
        visibleField[x][y] = view;
        revealFrom(x, y+1);
        revealFrom(x, y-1);
        revealFrom(x+1, y+1);
        revealFrom(x+1, y-1);
        revealFrom(x+1, y);
        revealFrom(x-1, y+1);
        revealFrom(x-1, y-1);
        revealFrom(x-1, y);
    }
}

//returns the number of mines around a square
int getMineNum(int x, int y)
{
    int ret = 0;

    ret += checkMine(x+0,y+1);
    ret += checkMine(x+0,y-1);
    ret += checkMine(x+1,y+1);
    ret += checkMine(x+1,y-1);
    ret += checkMine(x+1,y+0);
    ret += checkMine(x-1,y+1);
    ret += checkMine(x-1,y-1);
    ret += checkMine(x-1,y+0);

    return ret;
}

//does boundry checking for the minefield
int checkMine(int x, int y)
{
    if (x >= X_SIZE || x < 0 || y >= Y_SIZE || y < 0)
        return empty;
    else
        return minefield[x][y];
}

//checks if there is a mine in the location
//also does bounds checking
int isMine(int x, int y)
{
    if (x >= X_SIZE || x < 0 || y >= Y_SIZE || y < 0)
        return empty;
    else
    {
        return minefield[x][y];
    }
}

//checks if the player won
int isWin()
{
    int i, j;

    //every point that isn't a mine must be visible
    for (i = 0; i < Y_SIZE; i++)
    {
        for (j = 0; j < X_SIZE; j++)
        {
            if (minefield[j][i] != mine && visibleField[j][i] != view)
                return 0; //there is a point that is not visible and isn't a mine
        }
    }

    //every point with a mine must have a flag
    for (i = 0; i < Y_SIZE; i++)
    {
        for (j = 0; j < X_SIZE; j++)
        {
            if (minefield[j][i] == mine && flagField[j][i] != flag)
                return 0; //there is a point that has a mine, but no flag
        }
    }

    return 1; //all points check out, the player wins
}

int getPlayerInput()
{
    int x = -1;
    int y = -1;
    char df[1], inx[2], iny[2];


    printf("d to dig f to flag\nex- \"d 4 5\" digs at x=4, y=5\n");
    //fseek(stdin,0,SEEK_END); //uncomment for solaris system
    scanf("%1s %2s %2s", df, inx, iny);

    if (df[0] != 'd' && df[0] != 'f')
    {
        printf("Invalid input!\n");
        return 0;
    }

    x = atoi(inx) - 1;
    y = atoi(iny) - 1;

    if (x >= X_SIZE && x < 0)
    {
        printf("Invalid input!\n");
        return 0;
    }

    if (y >= Y_SIZE && y < 0)
    {
        printf("Invalid input!\n");
        return 0;
    }


    if (df[0] == 'd') {
        dig(x, y);
    } else if (df[0] == 'f'){
        return setFlag(x, y);
    } else {
        printf("Invalid input!\n");
        return 0;
    }
    return 1;
}

int setFlag(int x, int y)
{
    if (visibleField[x][y] == noView)
    {
        if (flagField[x][y] != flag)
        {
            flagField[x][y] = flag;
        }
        else
        {
            flagField[x][y] = noFlag;
        }

        return 1;
    }
    else
    {
        printf("Can't set a flag on a visible tile!\n");
        system("sleep 1");
        return 0;
    }
}