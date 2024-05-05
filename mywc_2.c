#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <libgen.h>
#include <ctype.h>

#define OPTION_ALL          0xffffffff
#define OPTION_LINE_COUNT   1
#define OPTION_WORD_COUNT   2
#define OPTION_FILE_SIZE    4

int option = 0;

/*
파일의 정보를 저장하는 구조체
*/
typedef struct wc_info
{
    char filename[1024];
    long  line_count;
    long  word_count;
    long  file_size;
} wc_info_t;

/*
* 파일을 분석하여 단어수, 라인수, 파일의 크기를 분석하는 함수
* 파라미터로 파일명을 넘기지 않고 FILE *를 넘긴 이유는 파일의 입력이 없는 경우
* 표준 입력에서 입력을 받기 때문임
*/
int word_count(FILE *fp, wc_info_t *info)
{
    char line[8096];
    char *tmp;
    int  size, idx;
    int  prev_space = 1;

    memset(info, 0x00, sizeof(wc_info_t));
    
    info->line_count == 1;
    while((size = fread(line, 1, sizeof(line), fp)) != 0) {
        for(idx = 0; idx < size; idx++) {
            /* 파일의 라인수를 계산하는 로직 */
            if(line[idx] == '\n') {
                info->line_count++;
            }

            /* 파일의 단어수를 계산하는 로직 */
            if(prev_space && !isspace(line[idx])) {
                info->word_count++;
                prev_space = 0;
            } else if(isspace(line[idx])) {
                prev_space = 1;
            }
        }
    }
    /* 파일 크기를 얻는 부분 */
    info->file_size = ftell(fp);
    if(info->file_size == 0) {
        info->line_count = 0;
    }
}

/* 
각각의 데이터의 합을 계산하고, 최대 큰 값의 데이터 폭을 return함 
*/
int summary(wc_info_t *sum, const wc_info_t *info, int count)
{
    int idx;
    memset(sum, 0x00, sizeof(wc_info_t));
    char size[32];
    
    strcpy(sum->filename, "total");
    for(idx = 0; idx < count; idx++) {
        sum->line_count += info[idx].line_count;
        sum->word_count += info[idx].word_count;
        sum->file_size  += info[idx].file_size;
    }
    if(option == OPTION_LINE_COUNT) {
        sprintf(size, "%ld", sum->line_count);
    } else if(option == OPTION_WORD_COUNT) {
        sprintf(size, "%ld", sum->word_count);
    } else {
        sprintf(size, "%ld", sum->file_size);
    }
    
    return strlen(size);
}

/* 
 option에 따라서 출력 여부가 결정되지만, 항상 라인수, 단어수, 파일크기 순으로 출력함 
*/
void print_info(const wc_info_t *info, int width)
{
    if(option == 0) {
        option = OPTION_ALL;
    }
    
    if(option & OPTION_LINE_COUNT) {
        printf("%*ld ", width, info->line_count);
    }
    if(option & OPTION_WORD_COUNT) {
        printf("%*ld ", width, info->word_count);
    }
    if(option & OPTION_FILE_SIZE) {
        printf("%*ld ", width, info->file_size);
    }
        
    printf("%s\n", info->filename);
}

void version(char *pgm)
{
    printf("%s version 1.0.0\n", basename(pgm));
}

void help(char *pgm)
{
    printf("Usage: %s [OPTIONS]... [FILES]...\n", basename(pgm));
    printf("line수, word수, file크기, 파일명을 출력하고 파일이 2개 이상이면 합계정보를 출력합니다.\n");
    printf("만약 파일을 입력하지 않으면 표준입력으로부터 데이터를 입력받습니다.\n");
    printf("  -c : 파일의 크기(byte수)를 출력합니다.\n");
    printf("  -l : 라인수를 출력합니다.\n");
    printf("  -w : 단어수를 출력합니다.\n");
    printf("  --help : 도움말을 출력하고 프로그램을 종료합니다.\n");
    printf("  --version : 버전정보를 출력하고 프로그램을 종료합니다.\n\n");
}

int check_arg(int argc, char **argv)
{
    int idx;
    int count = 0;
    
    for(idx = 1; idx < argc; idx++) {
        if(strcmp(argv[idx], "--help") == 0) {
            help(argv[0]);
            exit(0);
        } else if (strcmp(argv[idx], "--version") == 0) {
            version(argv[0]);
            exit(0);
        } else if(argv[idx][0] == '-') {
            int i;
            for (i = 1; argv[idx][i] != '\0'; i++) {
                if(argv[idx][i] == 'c') {
                    option |= OPTION_FILE_SIZE;
                } else if(argv[idx][i] == 'l') {
                    option |= OPTION_LINE_COUNT;
                } else if(argv[idx][i] == 'w') {
                    option |= OPTION_WORD_COUNT;
                } else {
                    fprintf(stderr, "invalid option: -%c\n", argv[idx][i]);
                    help(argv[0]);
                    exit(1);
                }
            }
        } else {
            count++;
        }
    }
    return count;
}

int main(int argc, char **argv)
{
    FILE *fp;
    wc_info_t *infos = NULL;
    wc_info_t sum;
    int  idx, width;
    int  file_count = 0;
    int  cur_file   = 0;

    file_count = argc;

     /* 파일이 없는 경우 에러 처리 */
    if(argc == 1) {
        fprintf(stderr, "No files specified.\n");
        help(argv[0]);
        exit(1);
    }
    
    if(argc > 1) {
        file_count = check_arg(argc, argv);
    }

    /* 파일이 없는 경우 에러 처리 */
    if(file_count == 0) {
        fprintf(stderr, "No files specified.\n");
        help(argv[0]);
        exit(1);
    }

    infos = (wc_info_t *)malloc(sizeof(wc_info_t) * file_count);
    for(idx = 1; idx < argc; idx++) {
        if(argv[idx][0] == '-') {
            continue;
        }
        if((fp = fopen(argv[idx], "r")) == NULL) {
            fprintf(stderr, "%s file open error: %s\n", argv[idx], strerror(errno));
            continue;
        }

        word_count(fp, &infos[cur_file]); 
        strcpy(infos[cur_file].filename, argv[idx]);
        cur_file++;

        fclose(fp);
    }

    width = summary(&sum, infos, cur_file);
    for(idx = 0; idx < cur_file; idx++) {
        print_info(&infos[idx], width);
    }
    if(cur_file > 1) {
        print_info(&sum, width);
    }
    free(infos);
}
