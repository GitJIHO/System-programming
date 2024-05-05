#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 4096
#define COPYMODE 0644
#define PATH_MAX 100

void oops(char *, char *);

int main(int ac, char *av[])
{
    int in_fd, out_fd, n_chars;
    char buf[BUFFERSIZE];
    char dest_path[PATH_MAX];

    if(ac != 3){
        fprintf(stderr, "usage: %s source destination\n", *av);
        exit(1);
    }
    if (strcmp(av[1], av[2]) == 0) {
        fprintf(stderr, "%s: '%s' and '%s' are the same file\n", av[0], av[1], av[2]);
        exit(1);
    }

    snprintf(dest_path, sizeof(dest_path), "/tmp/%s", av[1]);

    if((in_fd = open(av[1], O_RDONLY)) == -1)
        oops("Cannot open ", av[1]);

    if((out_fd = creat(dest_path, COPYMODE)) == -1)
        oops("Cannot create ", dest_path);

    while((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0)
        if(write(out_fd, buf, n_chars) != n_chars)
            oops("Write error to ", dest_path);

    if(n_chars == -1)
        oops("Read error from ", av[1]);

    if(close(in_fd) == -1 || close(out_fd) == -1)
        oops("Error closing files", "");

    return 0;
}

void oops(char *s1, char *s2)
{
    fprintf(stderr, "Error: %s ", s1);
    perror(s2);
    exit(1);
}
