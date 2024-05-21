#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <strings.h>

#define PORTNUM 18949
#define HOSTLEN 256
#define oops(msg) {perror(msg); exit(1);}

void sanitize(char *);

int main(int ac, char *av[])
{
    struct sockaddr_in saddr;
    struct hostent *hp;
    char hostname[HOSTLEN];
    int sock_id, sock_fd;
    FILE *sock_fpi, *sock_fpo;
    char dirname[BUFSIZ];
    int c;

    sock_id = socket(PF_INET, SOCK_STREAM, 0);
    if(sock_id == -1)
        oops("socket");

    bzero((void *)&saddr, sizeof(saddr));

    gethostname(hostname, HOSTLEN);
    hp = gethostbyname(hostname);

    bcopy((void *)hp->h_addr, (void *)&saddr.sin_addr, hp->h_length);
    saddr.sin_port = htons(PORTNUM);
    saddr.sin_family = AF_INET;

    if(bind(sock_id, (struct sockaddr *)&saddr, sizeof(saddr))!=0)
        oops("bind");
    
    if(listen(sock_id, 1) != 0)
        oops("listen");

    while(1){
        sock_fd = accept(sock_id, NULL, NULL);
        if(sock_fd == -1)
            oops("accept");
        if((sock_fpi = fdopen(sock_fd, "r")) == NULL)
            oops("fdopen reading");
        if(fgets(dirname, BUFSIZ-5, sock_fpi)==NULL)
            oops("reading dirname");
        sanitize(dirname);

        if((sock_fpo = fdopen(sock_fd, "w")) == NULL)
            oops("fdopen writing");

        // Fork a child process to execute the command
        int pid = fork();
        if (pid == -1) {
            perror("fork");
            fclose(sock_fpo);
            fclose(sock_fpi);
            close(sock_fd);
            continue;
        }

        if (pid == 0) { // Child process
            // Redirect stdout to sock_fpo
            if (dup2(fileno(sock_fpo), STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
            if (dup2(fileno(sock_fpo), STDERR_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
            fclose(sock_fpo);
            fclose(sock_fpi);
            close(sock_fd);

            execlp("ls", "ls", dirname, NULL);
            perror("execlp ls");
            exit(1);
        } else { // Parent process
            fclose(sock_fpo);
            fclose(sock_fpi);
            close(sock_fd);
            wait(NULL); // Wait for child to finish
        }
    }

    return 0;
}

void sanitize(char *str)
{
    char *src, *dest;

    for(src = dest = str ; *src ; src++)
        if(*src == '/' || isalnum(*src))
            *dest++ = *src;
    *dest = '\0';
}
