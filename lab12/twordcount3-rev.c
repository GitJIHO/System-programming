#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

struct arg_set {
    char *fname;
    int count;
};

void *count_words(void *);

int main(int ac, char *av[])
{
    if (ac < 2) {
        printf("usage: %s file1 [file2 ...]\n", av[0]);
        exit(1);
    }

    int num_files = ac - 1;
    pthread_t threads[num_files];
    struct arg_set args[num_files];
    int total_words = 0;

    for (int i = 0; i < num_files; i++) {
        args[i].fname = av[i + 1];
        args[i].count = 0;
        pthread_create(&threads[i], NULL, count_words, (void *)&args[i]);
    }

    for (int i = 0; i < num_files; i++) {
        pthread_join(threads[i], NULL);
        printf("%5d: %s\n", args[i].count, args[i].fname);
        total_words += args[i].count;
    }

    printf("%5d: total words\n", total_words);
    return 0;
}

void *count_words(void *a)
{
    struct arg_set *args = a;
    FILE *fp;
    int c, prevc = '\0';

    if ((fp = fopen(args->fname, "r")) != NULL) {
        while ((c = getc(fp)) != EOF) {
            if (!isalnum(c) && isalnum(prevc)) {
                args->count++;
            }
            prevc = c;
        }
        fclose(fp);
    } else {
        perror(args->fname);
    }
    return NULL;
}
