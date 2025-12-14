#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    // 参数的分类
    int file_num = 0;                                      // 文件个数
    int d_file_num = 0;                                    // 目录文件个数
    int o_file_num = 0;                                    // 普通文件个数
    int len = 0, num = 0;                                  // 选项个数
    char** file = (char**)malloc(argc * sizeof(char*));    // 存放文件路径
    char** d_file = (char**)malloc(argc * sizeof(char*));  // 存放目录文件路径
    char** o_file = (char**)malloc(argc * sizeof(char*));  // 存放普通文件路径
    char* parameter = (char*)malloc(1);                    // 存放选项
    bool a = false, l = false, R = false;                  // 选项
    bool t = false, r = false, s = false;                  // 选项
    bool I = false;  // 选项，为防止变量冲突，将i记为I
    for (int i = 1; i < argc; i++) {
        // 分别存放文件路径和选项
        if (argv[i][0] != '-') {
            // 存放文件路径
            file[file_num++] = argv[i];
            continue;
        }
        num += strlen(argv[i]) - 1;
        parameter = (char*)realloc(parameter, num + 1);
        for (int j = 1; j < strlen(argv[i]); j++) {
            // 存放选项
            if (argv[i][j] != 'a' && argv[i][j] != 'l' && argv[i][j] != 'R' &&
                argv[i][j] != 't' && argv[i][j] != 'r' && argv[i][j] != 'i' &&
                argv[i][j] != 's') {
                // 判断选项是否有效
                printf("ls: 无效的选项 -- %c\n", argv[i][j]);
                printf("请尝试执行 \"ls --help\" 来获取更多信息。\n");
                free(file);
                free(d_file);
                free(o_file);
                free(parameter);
                return 0;
            }
            parameter[len++] = argv[i][j];
        }
    }
    parameter[num] = '\0';
    // 文件路径的分类
    if (!file_num) {
        // 未输入文件路径的情况
        file[0] = ".";
        file_num = 1;
    }
    file[file_num] = NULL;
    for (int i = 0; i < file_num; i++) {
        struct stat statbuf;
        if (stat(file[i], &statbuf) == -1) {
            // 文件路径错误
            printf("ls: 无法访问 '%s': 没有那个文件或目录", file[i]);
        } else if (S_ISREG(statbuf.st_mode)) {
            // 普通文件路径
            o_file[o_file_num++] = file[i];
        } else {
            // 目录文件路径
            d_file[d_file_num++] = file[i];
        }
    }
    o_file[o_file_num] = NULL;
    d_file[d_file_num] = NULL;
    // 选项的处理
    for (int i = 0; i < num; i++) {
        if (parameter[i] == 'a') {
            a = true;
        } else if (parameter[i] == 'l') {
            l = true;
        } else if (parameter[i] == 'R') {
            R = true;
        } else if (parameter[i] == 't') {
            t = true;
        } else if (parameter[i] == 'r') {
            r = true;
        } else if (parameter[i] == 's') {
            s = true;
        } else {
            I = true;
        }
    }
    // 选项的执行
    for (int i = 0; i < file_num; i++) {
        struct stat statbuf;
        stat(file[i], &statbuf);
        if (S_ISREG(statbuf.st_mode)) {
            // 普通文件
            if (I) {
                // 参数 I(i) 的执行
                printf("%ld ", statbuf.st_ino);
            }
            printf("%s\n", file[i]);
        } else {
            // 目录文件
            DIR* dir = opendir(file[i]);
            struct dirent* dir_file;  // 目录下的文件
            while ((dir_file = readdir(dir)) != NULL) {
                // 参数 a 的执行
                if (!a && dir_file->d_name[0] == '.') {
                    continue;
                }
                struct stat d_statbuf;
                stat(dir_file->d_name, &d_statbuf);
                if (I) {
                    printf("%ld ", d_statbuf.st_ino);
                }
                printf("%s\n", dir_file->d_name);
            }
            closedir(dir);
        }
    }
    free(file);
    free(d_file);
    free(o_file);
    free(parameter);
    return 0;
}