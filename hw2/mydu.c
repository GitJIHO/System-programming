#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

//inode를 기록할 구조체
struct InodeRecord {
    ino_t inode;
    struct InodeRecord *next;
};

void print_size(long long blocks, int);

long long mydu(const char *, int, int, struct InodeRecord *);

int main(int ac, char *av[])
{
    int all = 0, bytes = 0;
    char *path[ac];
    int dirnum = 0;
    char *unique_paths[ac]; 
    int unique_path_count = 0;

    for (int i = 1; i < ac; i++)
    {
        if (strcmp(av[i], "-a") == 0 || strcmp(av[i], "-ab") == 0 || strcmp(av[i], "-ba") == 0)
        {
            all = 1;
        }
        if (strcmp(av[i], "-b") == 0 || strcmp(av[i], "-ab") == 0 || strcmp(av[i], "-ba") == 0)
        {
            bytes = 1;
        }
        else if (av[i][0] != '-')
        {
            int is_duplicate = 0;
            for (int j = 0; j < unique_path_count; j++) // 해당 Path가 list에 있는지 확인
            {
                if (strcmp(av[i], unique_paths[j]) == 0)
                {
                    is_duplicate = 1;
                    break;
                }
            }
            if (!is_duplicate) //duplicate하지 않으면, list에 추가
            {
                unique_paths[unique_path_count++] = av[i];
            }
        }
        else
        {
            if(all==0){
                printf("Use correct option -a, -b\n");
                exit(1);
            }
        }
    }

    if (unique_path_count == 0)
    {
        unique_paths[unique_path_count++] = ".";
    }

    for (int i = 0; i < unique_path_count; i++)
    {
        struct InodeRecord *inode_record_head = NULL;
        long long sum;
        struct stat st;
        if (stat(unique_paths[i], &st) == 0 && S_ISDIR(st.st_mode)) //해당 path가 dir인지 확인
        {
            sum = mydu(unique_paths[i], all, bytes, inode_record_head);
        }
        else
        {
            if (stat(unique_paths[i], &st) != 0)
            {
                fprintf(stderr, "Error: Cannot access %s\n", unique_paths[i]);
                continue;
            }
            sum = st.st_blocks;
        }

        if (sum == 0)
        {
            printf("There is no file or directory\n");
        }
        else
        {
            print_size(sum, bytes);
            printf("%s\n", unique_paths[i]);
        }
        struct InodeRecord *current = inode_record_head;
        while (current != NULL)
        {
            struct InodeRecord *temp = current;
            current = current->next;
            free(temp);
        }
    }

    return 0;
}

void print_size(long long blocks, int bytes)
{
    int i = 1;
    double s = (double)blocks / 2;

    if (bytes)
    {
        while (s >= 1024 && i < 3)
        {
            s /= 1024;
            i++;
        }
        printf("%lld\t", (long long)(s * (1LL << (10 * i))));
    }
    else
    {
        printf("%lld\t", (long long)s);
    }
}

long long mydu(const char *path, int all, int bytes, struct InodeRecord *inode_record_head)
{
    DIR *dirp;
    struct dirent *dp;

    if ((dirp = opendir(path)) == NULL)
        return 0;

    long long sum = 8;

    while ((dp = readdir(dirp)) != NULL)
    {
        struct stat st;
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, dp->d_name);

        if (lstat(filepath, &st) == -1 || dp->d_name[0] == '.')
        {
            continue;
        }

        struct InodeRecord *inode_ptr = inode_record_head;
        int already_printed = 0;
        while (inode_ptr != NULL)
        {
            if (inode_ptr->inode == st.st_ino)
            {
                already_printed = 1;
                break;
            }
            inode_ptr = inode_ptr->next;
        }

        if (!already_printed)
        {
            // 프린트되지 않았다면, inode를 추가해서 기록
            struct InodeRecord *new_inode_record = (struct InodeRecord *)malloc(sizeof(struct InodeRecord));
            new_inode_record->inode = st.st_ino;
            new_inode_record->next = inode_record_head;
            inode_record_head = new_inode_record;

            if (dp->d_type == DT_DIR && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
            {
                long long dirs = mydu(filepath, all, bytes, inode_record_head);
                sum += dirs;

                print_size(dirs, bytes);
                printf("%s\n", filepath);
            }
            else
            {
                sum += st.st_blocks;

                if (all)
                {
                    print_size(st.st_blocks, bytes);
                    printf("%s\n", filepath);
                }
            }
        }
    }

    closedir(dirp);

    return sum;
}
