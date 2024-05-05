#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

void print_size(long long blocks, int);

long long mydu(const char *, int, int, char **);

int main(int ac, char *av[]) {

    int all = 0, bytes = 0;
    char *path[ac];
    int dirnum = 0;

    for (int i = 1; i < ac; i++) {
        if (strcmp(av[i], "-a") == 0 || strcmp(av[i], "-ab") == 0 || strcmp(av[i], "-ba") == 0) {
            all = 1;
        }
        if (strcmp(av[i], "-b") == 0 || strcmp(av[i], "-ab") == 0 || strcmp(av[i], "-ba") == 0) {
            bytes = 1;
        }

        else if (av[i][0] != '-') {
            path[dirnum++] = av[i];
        }
        else {
            if(all==0){
                printf("Use correct option -a, -b\n");
                exit(1);
            }
        }
    }

    if (dirnum == 0) {
        path[dirnum++] = ".";
    }

    for (int i = 0; i < dirnum; i++) {
        char **printed_files = malloc(1024 * sizeof(char*)); // To keep track of printed files
        long long sum = mydu(path[i], all, bytes, printed_files);
        if (sum == 0) {
            printf("There is no file or directory\n");
        }
        else {
            print_size(sum, bytes);
            printf("%s\n", path[i]);
        }
        for (int j = 0; j < 1024; j++) {
            free(printed_files[j]);
        }
        free(printed_files);
    }

    return 0;
}

void print_size(long long blocks, int bytes) {
    int i = 1;
    double s = (double)blocks / 2;

    if (bytes) {
        while (s >= 1024 && i < 3) {
            s /= 1024;
            i++;
        }
        printf("%lld\t", (long long)(s * (1LL << (10 * i))));
    }
    else {
        printf("%lld\t", (long long)s);
    }
}

long long mydu(const char *path, int all, int bytes, char **printed_files) {
    DIR *dirp;
    struct dirent *dp;

    if ((dirp = opendir(path)) == NULL)
        return 0;

    long long sum = 8; // 8 is the size of directory
    int print_index = 0; // Index to keep track of printed files

    while ((dp = readdir(dirp)) != NULL) {
        struct stat st;
        char filepath[1024];

        snprintf(filepath, sizeof(filepath), "%s/%s", path, dp->d_name);

        if (stat(filepath, &st) == -1 || dp->d_name[0] == '.') {
            continue;
        }

        // Check if file path has already been printed
        int is_printed = 0;
        for (int i = 0; i < print_index; i++) {
            if (strcmp(printed_files[i], filepath) == 0) {
                is_printed = 1;
                break;
            }
        }
        if (is_printed) {
            continue; // Skip if file path has already been printed
        }

        // Store printed file path
        printed_files[print_index] = strdup(filepath);
        print_index++;

        if (dp->d_type == DT_DIR && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            long long dirs = mydu(filepath, all, bytes, printed_files);
            sum += dirs;

            if (all) {
                print_size(dirs, bytes);
                printf("%s\n", filepath);
            }
        }
        else {
            sum += st.st_blocks;

            if (all) {
                print_size(st.st_blocks, bytes);
                printf("%s\n", filepath);
            }
        }
    }

    closedir(dirp);

    return sum;
}
