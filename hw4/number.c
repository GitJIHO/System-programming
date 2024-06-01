#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define MAX_NUMBER 100

int game_running = 1;

void handle_signal(int sig) {
    game_running = 0;
}

void play_game(int read_fd) {
    int target;
    int guess;
    int attempts = 0;

    // 파이프에서 목표 숫자 읽기
    if (read(read_fd, &target, sizeof(target)) == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("숫자 맞추기 게임에 오신 것을 환영합니다!\n");
    printf("1부터 %d 사이의 숫자를 맞추세요.\n", MAX_NUMBER);

    while (game_running) {
        printf("추측한 숫자를 입력하세요: ");
        scanf("%d", &guess);
        attempts++;

        if (guess < target) {
            printf("더 큰 숫자입니다.\n");
        } else if (guess > target) {
            printf("더 작은 숫자입니다.\n");
        } else {
            printf("정답입니다! 시도 횟수: %d\n", attempts);
            break;
        }
    }

    if (!game_running) {
        printf("\n게임이 종료되었습니다.\n");
    }
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

        // 시그널 핸들러 설정
        signal(SIGINT, handle_signal);

        play_game(fd[0]);

        close(fd[0]); // 파이프의 읽기 끝 닫기
        wait(NULL); // 자식 프로세스 종료 대기
    }

    return 0;
}
