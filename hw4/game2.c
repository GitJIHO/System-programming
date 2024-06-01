#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define SNAKE_MAX_LENGTH 100
#define SNAKE_CHAR 'O'
#define FOOD_CHAR '*'

// 뱀의 위치를 저장할 구조체
typedef struct {
    int x, y;
} Position;

Position snake[SNAKE_MAX_LENGTH];
int snake_length = 1;
int food_x, food_y;
int max_x, max_y;
int game_running = 1;
int score = 0;
int fd[2]; // 파이프

void handle_signal(int sig) {
    game_running = 0;
}

void init_game() {
    initscr(); // ncurses 초기화
    cbreak(); // 줄 버퍼링 없이 문자 단위 입력
    noecho(); // 입력된 문자를 화면에 표시하지 않음
    keypad(stdscr, TRUE); // 특수 키 입력 허용
    curs_set(0); // 커서 감춤

    // 화면 크기 가져오기
    getmaxyx(stdscr, max_y, max_x);

    // 뱀의 초기 위치 설정
    snake[0].x = max_x / 2;
    snake[0].y = max_y / 2;

    // 음식의 초기 위치 설정
    food_x = rand() % max_x;
    food_y = rand() % max_y;

    // 시그널 핸들러 설정
    signal(SIGINT, handle_signal);
}

void end_game() {
    endwin(); // ncurses 종료
}

void draw_snake() {
    for (int i = 0; i < snake_length; i++) {
        mvprintw(snake[i].y, snake[i].x, "%c", SNAKE_CHAR);
    }
    mvprintw(food_y, food_x, "%c", FOOD_CHAR);
    refresh(); // 화면 갱신
}

void update_position(int ch) {
    Position new_head = snake[0];

    switch (ch) {
        case KEY_UP:
            new_head.y = (new_head.y > 0) ? new_head.y - 1 : max_y - 1;
            break;
        case KEY_DOWN:
            new_head.y = (new_head.y < max_y - 1) ? new_head.y + 1 : 0;
            break;
        case KEY_LEFT:
            new_head.x = (new_head.x > 0) ? new_head.x - 1 : max_x - 1;
            break;
        case KEY_RIGHT:
            new_head.x = (new_head.x < max_x - 1) ? new_head.x + 1 : 0;
            break;
    }

    // 음식과의 충돌 처리
    if (new_head.x == food_x && new_head.y == food_y) {
        snake_length++;
        score++;
        write(fd[1], &score, sizeof(score)); // 점수를 파이프에 쓰기
        food_x = rand() % max_x;
        food_y = rand() % max_y;
    }

    // 뱀의 몸 이동
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = new_head;
}

int main() {
    int ch;
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

    if (pid == 0) { // 자식 프로세스
        close(fd[1]); // 파이프의 쓰기 끝 닫기

        int read_score;
        while (game_running) {
            if (read(fd[0], &read_score, sizeof(read_score)) > 0) {
                printf("현재 점수: %d\n", read_score);
            }
        }
        close(fd[0]); // 파이프의 읽기 끝 닫기
        exit(EXIT_SUCCESS);
    } else { // 부모 프로세스
        close(fd[0]); // 파이프의 읽기 끝 닫기

        srand(time(NULL));
        init_game();

        while (game_running) {
            clear(); // 화면 지우기
            draw_snake(); // 뱀 그리기
            ch = getch(); // 키 입력 받기
            update_position(ch); // 위치 업데이트
            usleep(100000); // 게임 속도 조절
        }

        end_game();
        close(fd[1]); // 파이프의 쓰기 끝 닫기
    }

    return 0;
}
