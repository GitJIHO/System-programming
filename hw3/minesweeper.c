#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <signal.h>

#define NUM_mines 5 //지뢰의 수
#define HEIGHT 10 //높이
#define WIDTH 10 //너비

volatile sig_atomic_t stime;
bool lose = false;
bool win = false;
int mines = NUM_mines;
int Flag = NUM_mines;
int cursor_x, cursor_y;
int height = HEIGHT;
int width = WIDTH;

typedef struct Cell { //셀 구조체
    int number_of_mines;
    char ch;
    bool uncovered;
    bool bomb;
} Cell;

Cell cells[HEIGHT+1][WIDTH+1]; //셀 구조체 2차원 배열 생성

void sigint_handler(int);
void make_board();          
void print_board();        
void bombplacing_randomly(int);    
void numof_adjacent_mines();        
void uncover(int, int);    
void reveal_automatically(int, int);  
void check_for_win(int);   
void play_game();              

int main() {
    stime = time(NULL);
    signal(SIGQUIT, sigint_handler); //시그널 조절

    initscr();
    start_color(); //글자 색 변경 세팅용
    init_pair(1, COLOR_GREEN, COLOR_BLACK); //글자 색 변경 세팅용

    play_game();

    return 0;
}

void sigint_handler(int signum) { //signal 조절을 위한 함수
    getyx(stdscr, cursor_y, cursor_x); //경과시간 표시 전 입력 위치 기억
    time_t elapsed = time(NULL) - stime;
    mvprintw(0,0,"Elapsed time: %ld", elapsed);
    move(cursor_y, cursor_x); //경과 시간 표시 전 입력 위치로 이동
    refresh();
}

void make_board() { //게임 보드를 만드는 메소드
    int i, j;
    for (i = 0; i <= height + 1; ++i) {
        for (j = 0; j <= width + 1; ++j) {
            if (i == 0 || i == height + 1 || j == 0 || j == width + 1) {
                cells[i][j].bomb = false;
                cells[i][j].uncovered = true;
            }
            else {
                cells[i][j].ch = ' ';
                cells[i][j].number_of_mines = 0;
                cells[i][j].bomb = false;
                cells[i][j].uncovered = false;
            }
        }
    }
    return;
}

void print_board() { //변경된 게임판 값들을 출력하는 메소드
    clear();
    mvprintw(5, 0, "FLAGS: %d\n", Flag);
    attron(COLOR_PAIR(1));
    printw("+ ");
    int i, j;
    for (i = 0; i <= width-1; ++i)
        printw("%d ", i);
    printw("\n");
    for (i = 0; i <= height-1; ++i) {
        for (j = 0; j <= width; ++j) {
            if (j == 0){
                printw("%d ", i);
            } 
            else{
                attroff(COLOR_PAIR(1));
                if(cells[i+1][j].ch == '0'){
                    printw("X ");
                } else{
                    printw("%c ", cells[i+1][j].ch);
                }
            }
            attron(COLOR_PAIR(1));
        }
        printw("\n");
    }
    attron(COLOR_PAIR(1));
    mvprintw(17,0,"+ - - - - - - - - - - +");
    mvprintw(6,22,"+");
    for(int i=7; i<17; i++){
        mvprintw(i,22,"|");
    }
    attroff(COLOR_PAIR(1));   
    refresh();
    return;
}

void bombplacing_randomly(int mines) { //폭탄을 랜덤 배치하기 위한 메소드
    int random_row, random_col, num_of_mine = 0;
    while (num_of_mine < mines) {
        random_row = rand() % height; 
        random_col = rand() % width;  

        if (cells[random_row][random_col].bomb == false && (random_row != 0 && random_col != 0))
        {
            cells[random_row][random_col].bomb = true;
            num_of_mine++;
        }
    }
    return;
}

void numof_adjacent_mines() { //셀 근처의 폭탄 개수를 파악하기 위한 메소드
    int i, j, m, n;
    for (i = 1; i <= height; ++i) {
        for (j = 1; j <= width; ++j) {
            if (cells[i][j].bomb == false) {
                for (m = i - 1; m <= i + 1; ++m)
                    for (n = j - 1; n <= j + 1; ++n)
                        if (cells[m][n].bomb == true)
                            cells[i][j].number_of_mines++;
            }
        }
    }
    return;
}

void uncover(int a, int b) { //셀을 클릭했을 때를 처리하는 메소드
    if (cells[a][b].bomb == true) { //폭탄을 클릭했을 때 lose
        lose = true;
        clear();
        mvprintw(5,17,"YOU LOSE!");
        for(int i=9; i>0; i--){
            mvprintw(8,5,"PROGRAM WILL BE TERMINATED IN ... %d", i);
            refresh();
            sleep(1);
        }
        mvprintw(8,5,"PROGRAM WILL BE TERMINATED IN ... %d", 0);
        refresh();
        clear();
        return;
    }
    if(cells[a][b].ch == 'F' && cells[a][b].bomb == false){ //미리 Flag한것이 uncover되는 경우
        Flag++;
    }
    cells[a][b].ch = cells[a][b].number_of_mines + '0';
    if (cells[a][b].number_of_mines == 0){ //지뢰가 없는 경우
        reveal_automatically(a, b); //주변 다시 탐색
    } else
        cells[a][b].uncovered = true;

    return;
}

void reveal_automatically(int a, int b) { //지뢰가 없는 경우 주변을 다시 탐색하는 메소드
    int i, j;
    if (cells[a][b].uncovered == false) {
        cells[a][b].uncovered = true;
        for (i = a - 1; i <= a + 1; ++i)
            for (j = b - 1; j <= b + 1; ++j)
                if (cells[i][j].uncovered == false)
                    uncover(i, j);
    }
    return;
}

void check_for_win(int mines) { //게임 승리 여부를 판단하는 메소드
    int i, j, counter = 0;

    for (i = 1; i <= height; ++i)
        for (j = 1; j <= width; ++j){
            if (cells[i][j].bomb == true && cells[i][j].ch == 'F')
                counter++;
        }

    if (counter == mines) { //승리시
        win = true;
        clear();
        mvprintw(5,17,"YOU WIN!");
        for(int i=9; i>0; i--){
            mvprintw(8,5,"PROGRAM WILL BE TERMINATED IN ... %d", i);
            refresh();
            sleep(1);
        }
        mvprintw(8,5,"PROGRAM WILL BE TERMINATED IN ... %d", 0);
        refresh();
        clear();
        return;
    }
}

void play_game() { //게임 전체 진행을 위한 메소드

    srand(time(NULL)); //폭탄 위치 랜덤값 설정을 위한 세팅

    int x, y, row, column;
    char op;

    make_board();
    bombplacing_randomly(mines);
    numof_adjacent_mines();

    do {
        print_board();

        int trow = 18;
        int trol = 0;
        mvprintw(trow,trol,"X: ");
        mvprintw(trow+1,trol,"Y: ");
        mvprintw(trow+2,trol,"A: ");
        mvscanw(trow,trol+3,"%d", &column);
        mvscanw(trow+1,trol+3,"%d", &row);
        mvscanw(trow+2,trol+3,"%c", &op);
        if(row<0 || column<0){
            continue;
        }
        row++;
        column++;
        if (op == 'c' || op == 'C'){
            if(cells[row][column].ch != 'F'){
                uncover(row, column);
            }
        }
        if (op == 'f' || op == 'F'){
            if(cells[row][column].ch != 'F' && cells[row][column].ch == ' ' && Flag>0){
                cells[row][column].ch = 'F';
                Flag--;
            }
        }
        if (op == 'x' || op == 'X'){
            if(cells[row][column].ch == 'F'){
                cells[row][column].ch = ' ';
                Flag++;
            }
        }
        if (!lose)
            check_for_win(mines);
    } while (!lose && !win);
    
    return;
}