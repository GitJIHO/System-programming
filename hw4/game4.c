#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define BOARD_SIZE 5
#define NUM_SHIPS 3

typedef struct {
    int x;
    int y;
} Position;

void initialize_board(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = '~'; // Water
        }
    }
}

void place_ships(char board[BOARD_SIZE][BOARD_SIZE]) {
    srand(time(NULL));
    for (int i = 0; i < NUM_SHIPS; i++) {
        int x = rand() % BOARD_SIZE;
        int y = rand() % BOARD_SIZE;
        board[x][y] = 'S'; // Ship
    }
}

void draw_board(WINDOW *win, char board[BOARD_SIZE][BOARD_SIZE], int offset_x, int offset_y) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            mvwprintw(win, offset_y + i, offset_x + j * 2, "%c", board[i][j]);
        }
    }
    wrefresh(win);
}

int is_hit(char board[BOARD_SIZE][BOARD_SIZE], int x, int y) {
    return board[x][y] == 'S';
}

void mark_hit(char board[BOARD_SIZE][BOARD_SIZE], int x, int y) {
    board[x][y] = 'X';
}

void mark_miss(char board[BOARD_SIZE][BOARD_SIZE], int x, int y) {
    board[x][y] = 'O';
}

int main() {
    int fd[2];
    pid_t pid;

    // 파이프 생성
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // 자식 프로세스 (컴퓨터)
        close(fd[0]); // 파이프의 읽기 끝 닫기

        char computer_board[BOARD_SIZE][BOARD_SIZE];
        initialize_board(computer_board);
        place_ships(computer_board);

        // 컴퓨터 보드 전송
        if (write(fd[1], computer_board, sizeof(computer_board)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        close(fd[1]); // 파이프의 쓰기 끝 닫기
        exit(EXIT_SUCCESS);
    } else { // 부모 프로세스 (플레이어)
        close(fd[1]); // 파이프의 쓰기 끝 닫기

        char player_board[BOARD_SIZE][BOARD_SIZE];
        char computer_board[BOARD_SIZE][BOARD_SIZE];
        initialize_board(player_board);
        place_ships(player_board);

        // 컴퓨터 보드 받기
        if (read(fd[0], computer_board, sizeof(computer_board)) == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        close(fd[0]); // 파이프의 읽기 끝 닫기

        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);

        WINDOW *player_win = newwin(BOARD_SIZE + 2, BOARD_SIZE * 2 + 2, 0, 0);
        WINDOW *computer_win = newwin(BOARD_SIZE + 2, BOARD_SIZE * 2 + 2, 0, BOARD_SIZE * 2 + 4);

        box(player_win, 0, 0);
        box(computer_win, 0, 0);
        mvwprintw(player_win, 0, 2, "Player");
        mvwprintw(computer_win, 0, 2, "Computer");

        draw_board(player_win, player_board, 1, 1);
        draw_board(computer_win, computer_board, 1, 1);

        int game_over = 0;
        int x, y;
        char input[10] = {0};
        int input_pos = 0;

        while (!game_over) {
            mvprintw(BOARD_SIZE + 2, 0, "Enter attack coordinates (format: x y): ");
            refresh();

            echo();
            scanw("%d %d", &x, &y);
            noecho();

            if (is_hit(computer_board, x, y)) {
                mark_hit(computer_board, x, y);
                mvprintw(BOARD_SIZE + 3, 0, "Hit!");
            } else {
                mark_miss(computer_board, x, y);
                mvprintw(BOARD_SIZE + 3, 0, "Miss.");
            }

            draw_board(computer_win, computer_board, 1, 1);

            // 컴퓨터의 공격
            int cx = rand() % BOARD_SIZE;
            int cy = rand() % BOARD_SIZE;
            if (is_hit(player_board, cx, cy)) {
                mark_hit(player_board, cx, cy);
                mvprintw(BOARD_SIZE + 4, 0, "Computer hit at (%d, %d)!", cx, cy);
            } else {
                mark_miss(player_board, cx, cy);
                mvprintw(BOARD_SIZE + 4, 0, "Computer miss at (%d, %d).", cx, cy);
            }

            draw_board(player_win, player_board, 1, 1);

            // 게임 종료 조건 체크
            int player_ships_left = 0;
            int computer_ships_left = 0;
            for (int i = 0; i < BOARD_SIZE; i++) {
                for (int j = 0; j < BOARD_SIZE; j++) {
                    if (player_board[i][j] == 'S') player_ships_left++;
                    if (computer_board[i][j] == 'S') computer_ships_left++;
                }
            }

            if (player_ships_left == 0) {
                mvprintw(BOARD_SIZE + 5, 0, "Computer wins!");
                game_over = 1;
            } else if (computer_ships_left == 0) {
                mvprintw(BOARD_SIZE + 5, 0, "Player wins!");
                game_over = 1;
            }

            refresh();
        }

        mvprintw(BOARD_SIZE + 6, 0, "Press any key to exit.");
        refresh();
        getch();

        endwin();
    }

    return 0;
}
