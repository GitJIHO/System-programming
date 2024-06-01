#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define MAX_NUMBER 100

void play_game(int read_fd) {
    int target;
    int guess;
    int attempts = 0;
    int ch;
    char input[10] = {0};
    int input_pos = 0;

    // 파이프에서 목표 숫자 읽기
    if (read(read_fd, &target, sizeof(target)) == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    // setlocale(LC_ALL, "");


    mvprintw(0, 0, "숫자 맞추기 게임에 오신 것을 환영합니다!");
    mvprintw(1, 0, "1부터 %d 사이의 숫자를 맞추세요.", MAX_NUMBER);
    mvprintw(3, 0, "추측한 숫자를 입력하세요: ");
    refresh();

    while (1) {
        ch = getch();
        if (ch == '\n') {
            guess = atoi(input);
            attempts++;
            input_pos = 0;
            memset(input, 0, sizeof(input));

            if (guess < target) {
                mvprintw(4, 0, "더 큰 숫자입니다.");
            } else if (guess > target) {
                mvprintw(4, 0, "더 작은 숫자입니다.");
            } else {
                mvprintw(4, 0, "정답입니다! 시도 횟수: %d", attempts);
                mvprintw(5, 0, "게임을 종료하려면 아무 키나 누르세요.");
                refresh();
                getch();
                break;
            }
            mvprintw(3, 0, "추측한 숫자를 입력하세요: ");
            refresh();
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (input_pos > 0) {
                input_pos--;
                input[input_pos] = '\0';
                mvprintw(3, 0, "추측한 숫자를 입력하세요: %s", input);
                refresh();
            }
        } else if (ch >= '0' && ch <= '9') {
            if (input_pos < sizeof(input) - 1) {
                input[input_pos++] = ch;
                mvprintw(3, 0, "추측한 숫자를 입력하세요: %s", input);
                refresh();
            }
        }
    }

    endwin();
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

    if (pid == 0) { // 자식 프로세스
        close(fd[0]); // 파이프의 읽기 끝 닫기

        // 난수 생성기 초기화
        srand(time(NULL));
        int target = rand() % MAX_NUMBER + 1;

        // 목표 숫자를 파이프에 씀
        if (write(fd[1], &target, sizeof(target)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        close(fd[1]); // 파이프의 쓰기 끝 닫기
        exit(EXIT_SUCCESS);
    } else { // 부모 프로세스
        close(fd[1]); // 파이프의 쓰기 끝 닫기

        play_game(fd[0]);

        close(fd[0]); // 파이프의 읽기 끝 닫기
        wait(NULL); // 자식 프로세스 종료 대기
    }

    return 0;
}
