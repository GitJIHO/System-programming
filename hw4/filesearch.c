#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 1024

void search_keyword_in_file(const char *filename, const char *keyword);

int main() {
    char filename[256];
    char keyword[256];
    
    // 사용자로부터 파일 경로와 검색 키워드를 입력받음
    printf("파일 경로를 입력하세요: ");
    scanf("%s", filename);
    printf("검색할 키워드를 입력하세요: ");
    scanf("%s", keyword);
    
    search_keyword_in_file(filename, keyword);
    
    return 0;
}

void search_keyword_in_file(const char *filename, const char *keyword) {
    int fd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    int file;
    ssize_t bytes_read;
    
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
        
        file = open(filename, O_RDONLY);
        if (file == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        
        while ((bytes_read = read(file, buffer, BUFFER_SIZE)) > 0) {
            if (write(fd[1], buffer, bytes_read) != bytes_read) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
        
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        close(file);
        close(fd[1]); // 파이프의 쓰기 끝 닫기
        exit(EXIT_SUCCESS);
    } else { // 부모 프로세스
        close(fd[1]); // 파이프의 쓰기 끝 닫기
        
        while ((bytes_read = read(fd[0], buffer, BUFFER_SIZE)) > 0) {
            buffer[bytes_read] = '\0';
            if (strstr(buffer, keyword) != NULL) {
                printf("키워드 '%s'가 파일 내에 발견되었습니다.\n", keyword);
                break;
            }
        }
        
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        close(fd[0]); // 파이프의 읽기 끝 닫기
    }
}
