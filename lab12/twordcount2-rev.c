#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

int total_words;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

void *count_words(void *);

int main(int ac, char *av[])
{
    if (ac < 2) {
        printf("usage: %s file1 [file2 ...]\n", av[0]);
        exit(1);
    }

    int num_files = ac - 1;
    pthread_t threads[num_files];
    total_words = 0;

    for (int i = 0; i < num_files; i++) {
        pthread_create(&threads[i], NULL, count_words, (void *)av[i + 1]);
    }

    for (int i = 0; i < num_files; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("%5d: total words\n", total_words);
    return 0;
}

void *count_words(void *f)
{
    char *filename = (char *)f;
    FILE *fp;
    int c, prevc = '\0';

    if ((fp = fopen(filename, "r")) != NULL) {
        while ((c = getc(fp)) != EOF) {
            if (!isalnum(c) && isalnum(prevc)) {
                pthread_mutex_lock(&counter_lock);
                total_words++;
                pthread_mutex_unlock(&counter_lock);
            }
            prevc = c;
        }
        fclose(fp);
    } else {
        perror(filename);
    }
    return NULL;
}
