#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>

#define MAX_DIGITS 4
#define READ_END 0
#define WRITE_END 1

void generate_secret_number(int *secret_number) {
    srand(time(NULL));
    int chosen[10] = {0}; // 0부터 9까지의 숫자가 선택되었는지 여부를 나타내는 배열

    for (int i = 0; i < MAX_DIGITS; i++) {
        int digit;
        do {
            digit = rand() % 10; // 0부터 9까지의 난수 생성
        } while (chosen[digit]); // 이미 선택된 숫자라면 다시 선택

        secret_number[i] = digit;
        chosen[digit] = 1; // 선택된 숫자 표시
    }
}

void compare_numbers(int *secret_number, int *guess, int *result) {
    for (int i = 0; i < MAX_DIGITS; i++) {
        result[i] = 0;
        if (secret_number[i] == guess[i]) {
            result[i] = 2; // 숫자와 위치가 일치함 (스트라이크)
        } else {
            for (int j = 0; j < MAX_DIGITS; j++) {
                if (secret_number[i] == guess[j]) {
                    result[i] = 1; // 숫자는 일치하지만 위치가 일치하지 않음 (볼)
                    break;
                }
            }
        }
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    int secret_number[MAX_DIGITS];
    generate_secret_number(secret_number);

    printf("숫자 야구 게임을 시작합니다.\n");
    printf("컴퓨터가 비밀 숫자를 선택했습니다. 숫자를 맞춰보세요!\n\n");

    int fd[2];
    if (pipe(fd) == -1) {
        fprintf(stderr, "Pipe failed");
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "fork failed\n");
        return 1;
    } else if (pid == 0) { // 자식 프로세스
        close(fd[WRITE_END]); // 자식 프로세스는 쓰기용 파이프 닫음

        int guess[MAX_DIGITS];
        int result[MAX_DIGITS];

        read(fd[READ_END], secret_number, MAX_DIGITS * sizeof(int)); // 부모 프로세스로부터 비밀 숫자 읽기

        while (1) {
            char guess_str[MAX_DIGITS + 1];
            printf("숫자를 입력하세요 (4자리): ");
            scanf("%s", guess_str);

            bool valid_input = true;
            for (int i = 0; i < MAX_DIGITS; i++) { 
                if (guess_str[i] < '0' || guess_str[i] > '9') { //범위에 벗어나는 경우 판별
                    valid_input = false;
                    break;
                }
                guess[i] = guess_str[i] - '0'; //정수형으로 저장함
            }

            if (!valid_input) {
                printf("잘못된 입력입니다. 0부터 9까지의 숫자를 4자리 입력하세요.\n\n");
                clear_input_buffer(); //버퍼를 비워줌
                continue;
            }

            compare_numbers(secret_number, guess, result);

            int strikes = 0;
            int balls = 0;

            for (int i = 0; i < MAX_DIGITS; i++) {
                if (result[i] == 2) {
                    strikes++;
                } else if (result[i] == 1) {
                    balls++;
                }
            }
            if (strikes == MAX_DIGITS) {
                printf("정답입니다! 비밀 숫자를 맞췄습니다.\n");
                break;
            } else if (strikes == 0 && balls == 0) {
                printf("아웃입니다! 숫자를 다시 추측하세요.\n\n");
            } else {
                printf("%d스트라이크, %d볼입니다.\n\n", strikes, balls);
            }
        }

        close(fd[READ_END]); // 읽기용 파이프 닫음
    } else { // 부모 프로세스
        close(fd[READ_END]); // 부모 프로세스는 읽기용 파이프 닫음

        write(fd[WRITE_END], secret_number, MAX_DIGITS * sizeof(int)); // 비밀 숫자를 자식 프로세스에게 전달

        wait(NULL);
        printf("게임 종료\n");

        close(fd[WRITE_END]); // 쓰기용 파이프 닫음
    }

    return 0;
}
