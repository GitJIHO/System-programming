/*
시스템프로그래밍_hw1
학번: 2022428947
이름: 이지호
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

#define BUF_SIZE 4096

int count_lines(char *filename) { //옵션 l 수행을 위한 작업
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename); //파일 오픈에 문제가 있을 때(ex. 읽기 권한이 없거나 등등..)
        exit(1);
    }
    int count = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            count++;
        }
    }
    fclose(file);
    return count;
}

int count_words(char *filename) { //옵션 w 수행을 위한 작업
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(1);
    }
    int count = 0;
    bool in_word = false;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == ' ' || ch == '\n' || ch == '\t') {
            if (in_word) {
                count++;
                in_word = false;
            }
        } else {
            in_word = true;
        }
    }
    if (in_word) {
        count++;
    }
    fclose(file);
    return count;
}

int count_bytes(char *filename) { //옵션 c 수행을 위한 작업
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(1);
    }
    int count = 0;
    char buffer[BUF_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, BUF_SIZE)) > 0) {
        count += bytes_read;
    }
    close(fd);
    return count;
}

int main(int argc, char *argv[]) {
    int opt;
    bool opt_l = false, opt_w = false, opt_c = false;

    for(int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0) {
                printf("%s version 1.0.0\n", argv[0]); //version은 1.0.0으로 출력하겠습니다
                exit(0);
        }
    }

    while ((opt = getopt(argc, argv, "lwc")) != -1) { //옵션이 뭐가 들어왔는지 확인
        switch (opt) {
            case 'l':
                opt_l = true;
                break;
            case 'w':
                opt_w = true;
                break;
            case 'c':
                opt_c = true;
                break;
            default: //옵션값이 lwc중에 없는 경우
                fprintf(stderr, "Try '%s --help' for more information.\n", *argv);
                //공지사항을 참고하여 help는 따로 구현하지 않았습니다
                exit(1);
        }
    }

    if (!opt_l && !opt_w && !opt_c) {
        opt_l = opt_w = opt_c = true;
    }//옵션이 없는 경우 옵션을 모두 수행

    if (optind == argc) {
        fprintf(stderr, "Usage: %s [-l | -w | -c] <filename1>\n", *argv);
        exit(1);
    }//매개변수를 전달하지 않는 경우

    int total_lines = 0, total_words = 0, total_bytes = 0;
    bool multiple_files = optind < argc - 1; //파일이 2개 이상인경우를 판별

    for (int i = optind; i < argc; i++) {
        char *filename = argv[i];
        if (access(filename, F_OK) == -1) {
            fprintf(stderr,  "%s: %s: No such file of directory\n", *argv, filename);
            //폴더에 파일이 존재하지 않는 경우
            continue;
        }

        int lines = 0, words = 0, bytes = 0;
        if (opt_l) {
            lines = count_lines(filename);
            printf("%d ", lines);
        }
        if (opt_w) {
            words = count_words(filename);
            printf("%d ", words);
        }
        if (opt_c) {
            bytes = count_bytes(filename);
            printf("%d ", bytes);
        }
        if (opt_l || opt_w || opt_c) {
            printf("%s\n", filename);
        }
        //결과 출력

        total_lines += lines;
        total_words += words;
        total_bytes += bytes;
        //total값을 계산하기 위한 작업
    }

    if (multiple_files) { //파일이 2개 이상인 경우의 total값 출력
        if(opt_l && opt_w && opt_c){
            printf("%d %d %d total\n", total_lines, total_words, total_bytes);
        }
        else if(opt_l && opt_w){
            printf("%d %d total\n", total_lines, total_words);           
        }
        else if(opt_l && opt_c){
            printf("%d %d total\n", total_lines, total_bytes);           
        }
        else if(opt_w && opt_c){
            printf("%d %d total\n", total_words, total_bytes);           
        }
        else if(opt_l){
            printf("%d total\n", total_lines);                      
        }
        else if(opt_w){
            printf("%d total\n", total_words);                      
        }
        else{
            printf("%d total\n", total_bytes);                      
        }
    }

    return 0;
}
