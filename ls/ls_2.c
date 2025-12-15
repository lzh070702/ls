#define _DEFAULT_SOURCE
#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <locale.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// 选项

bool a = false, l = false, R = false;
bool t = false, r = false, s = false;
bool I = false;  // 选项，为防止变量冲突，将i记为I

// 文件路径的排序

int file_qsort(const void* A, const void* B) {
    const char* cmp_a = *(const char**)A;
    const char* cmp_b = *(const char**)B;
    // .和..排序
    if (!strcmp(cmp_a, ".")) {
        return -1;
    } else if (!strcmp(cmp_b, ".")) {
        return 1;
    }
    if (!strcmp(cmp_a, "..")) {
        return -1;
    } else if (!strcmp(cmp_b, "..")) {
        return 1;
    }
    // 文件名排序
    int str_a = (cmp_a[0] == '.') ? 1 : 0;
    int str_b = (cmp_b[0] == '.') ? 1 : 0;
    while (cmp_a[str_a] && cmp_b[str_b]) {
        if (cmp_a[str_a] < 0 || cmp_b[str_b] < 0) {
            // 中英排序
            return cmp_a[str_a] - cmp_b[str_b];
        }
        int lower_a = tolower((unsigned char)cmp_a[str_a]);
        int lower_b = tolower((unsigned char)cmp_b[str_b]);
        if (lower_a != lower_b) {
            // 字母顺序排序
            return lower_a - lower_b;
        }
        str_a++;
        str_b++;
    }
    if (cmp_a[str_a] != cmp_b[str_b]) {
        // str已经到'\0'（短的）排在前面
        return cmp_a[str_a] - cmp_b[str_b];
    }
    // 字母大小写排序
    str_a = (cmp_a[0] == '.') ? 1 : 0;
    str_b = (cmp_b[0] == '.') ? 1 : 0;
    while (cmp_a[str_a] && cmp_b[str_b]) {
        // 字母大小写排序
        if (islower(cmp_a[str_a]) && isupper(cmp_b[str_b])) {
            return -1;
        }
        if (isupper(cmp_a[str_a]) && islower(cmp_b[str_b])) {
            return 1;
        }
        str_a++;
        str_b++;
    }
    // 隐藏文件排序
    int hidden_a = (cmp_a[0] == '.');
    int hidden_b = (cmp_b[0] == '.');
    if (hidden_a && !hidden_b) {
        return -1;
    }
    if (!hidden_a && hidden_b) {
        return 1;
    }
    return 0;
}

// 文件路径的排序-r

int r_file_qsort(const void* A, const void* B) {
    return file_qsort(B, A);
}

// 文件路径的排序-t

int t_file_qsort(const void* A, const void* B) {
    const char* cmp_a = *(const char**)A;
    const char* cmp_b = *(const char**)B;
    struct stat statbuf_a, statbuf_b;
    lstat(cmp_a, &statbuf_a);
    lstat(cmp_b, &statbuf_b);
    return statbuf_b.st_mtime - statbuf_a.st_mtime;
}

// 文件路径的排序-tr

int tr_file_qsort(const void* A, const void* B) {
    return t_file_qsort(B, A);
}

// 目录下文件的排序

int scandir_sort(const struct dirent** A, const struct dirent** B) {
    const char* name_a = (*A)->d_name;
    const char* name_b = (*B)->d_name;
    return file_qsort(&name_a, &name_b);
}

// 目录下文件的排序-r

int r_scandir_sort(const struct dirent** A, const struct dirent** B) {
    return scandir_sort(B, A);
}

// 目录下文件的排序-t

int t_scandir_sort(const struct dirent** A, const struct dirent** B) {
    const char* name_a = (*A)->d_name;
    const char* name_b = (*B)->d_name;
    return t_file_qsort(&name_a, &name_b);
}

// 目录下文件的排序-tr

int tr_scandir_sort(const struct dirent** A, const struct dirent** B) {
    return t_scandir_sort(B, A);
}

// 选项 R 的实现

void ls_print(char* DF) {
    if (access(DF, R_OK) != 0) {
        printf("ls: 无法打开目录 '%s': 权限不够\n", DF);
    } else {
        printf("\n%s:\n", DF);
        struct dirent** d_file;  // 目录下文件的结构体指针数组
        int d_file_num = 0;
        // 目录下文件排序
        if (!t && !r) {
            d_file_num = scandir(DF, &d_file, NULL, scandir_sort);
        } else if (!t && r) {
            d_file_num = scandir(DF, &d_file, NULL, r_scandir_sort);
        } else if (t && !r) {
            d_file_num = scandir(DF, &d_file, NULL, t_scandir_sort);
        } else {
            d_file_num = scandir(DF, &d_file, NULL, tr_scandir_sort);
        }
        if (l || s) {
            // 计算目录下文件的总块数并打印
            blkcnt_t st_blocks = 0;
            for (int j = 0; j < d_file_num; j++) {
                if (!a && d_file[j]->d_name[0] == '.') {
                    continue;
                }
                struct stat d_statbuf;
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", DF,
                         d_file[j]->d_name);
                lstat(full_path, &d_statbuf);
                st_blocks += (d_statbuf.st_blocks / 2);
            }
            printf("总计 ");
            printf("%lu\n", (unsigned long)st_blocks);
        }
        for (int j = 0; j < d_file_num; j++) {
            if (!a && d_file[j]->d_name[0] == '.') {
                continue;
            }
            struct stat d_statbuf;
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", DF,
                     d_file[j]->d_name);
            lstat(full_path, &d_statbuf);
            if (I) {
                printf("%7lu ", (unsigned long)d_statbuf.st_ino);
            }
            if (s) {
                printf("%4lu ", (unsigned long)d_statbuf.st_blocks / 2);
            }
            if (l) {
                putchar(S_ISREG(d_statbuf.st_mode) ? '-' : 'd');
                putchar((d_statbuf.st_mode & S_IRUSR) ? 'r' : '-');
                putchar((d_statbuf.st_mode & S_IWUSR) ? 'w' : '-');
                putchar((d_statbuf.st_mode & S_IXUSR) ? 'x' : '-');
                putchar((d_statbuf.st_mode & S_IRGRP) ? 'r' : '-');
                putchar((d_statbuf.st_mode & S_IWGRP) ? 'w' : '-');
                putchar((d_statbuf.st_mode & S_IXGRP) ? 'x' : '-');
                putchar((d_statbuf.st_mode & S_IROTH) ? 'r' : '-');
                putchar((d_statbuf.st_mode & S_IWOTH) ? 'w' : '-');
                putchar((d_statbuf.st_mode & S_IXOTH) ? 'x' : '-');
                printf(" %2lu ", (unsigned long)d_statbuf.st_nlink);
                printf("%s ", getpwuid(d_statbuf.st_uid)->pw_name);
                printf("%s ", getgrgid(d_statbuf.st_gid)->gr_name);
                printf("%6lu ", (unsigned long)d_statbuf.st_size);
                // mtime
                struct tm* local_time = localtime(&d_statbuf.st_mtime);
                setlocale(LC_TIME, "zh_CN.UTF-8");
                char time_str[32];
                strftime(time_str, sizeof(time_str), "%m月 %d %H:%M",
                         local_time);
                printf("%s ", time_str);
            }
            printf("%s\n", d_file[j]->d_name);  // 颜色
        }
        for (int j = 2; j < d_file_num; j++) {
            if (!a && d_file[j]->d_name[0] == '.') {
                continue;
            }
            struct stat d_statbuf;
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", DF,
                     d_file[j]->d_name);
            lstat(full_path, &d_statbuf);
            if (S_ISDIR(d_statbuf.st_mode)) {
                ls_print(full_path);
            }
            free(d_file[j]);
        }
    }
}

int main(int argc, char* argv[]) {
    // 参数的分类

    int file_num = 0;                                     // 文件个数
    int ofile_num = 0;                                    // 普通文件个数
    int dfile_num = 0;                                    // 目录文件个数
    int len = 0, num = 0;                                 // 选项个数
    char** file = (char**)malloc(argc * sizeof(char*));   // 存放文件路径
    char** ofile = (char**)malloc(argc * sizeof(char*));  // 存放普通文件路径
    char** dfile = (char**)malloc(argc * sizeof(char*));  // 存放目录文件路径
    char* parameter = (char*)malloc(1);                   // 存放选项

    for (int i = 1; i < argc; i++) {
        // 分别存放文件路径和选项
        if (argv[i][0] != '-' || (strlen(argv[i]) == 1)) {
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
                free(dfile);
                free(ofile);
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
        if (lstat(file[i], &statbuf) == -1) {
            // 文件路径错误
            printf("ls: 无法访问 '%s': 没有那个文件或目录\n", file[i]);
        } else if (S_ISREG(statbuf.st_mode)) {
            // 普通文件路径
            ofile[ofile_num++] = file[i];
        } else {
            // 目录文件路径
            dfile[dfile_num++] = file[i];
        }
    }
    ofile = (char**)realloc(ofile, (ofile_num + 1) * sizeof(char*));
    dfile = (char**)realloc(dfile, (dfile_num + 1) * sizeof(char*));
    ofile[ofile_num] = NULL;
    dfile[dfile_num] = NULL;

    // 选项的处理

    for (int i = 0; i < num; i++) {
        switch (parameter[i]) {
            case 'a':
                a = true;
                break;
            case 'l':
                l = true;
                break;
            case 'R':
                R = true;
                break;
            case 't':
                t = true;
                break;
            case 'r':
                r = true;
                break;
            case 's':
                s = true;
                break;
            case 'i':
                I = true;
                break;
        }
    }

    // 文件的排序

    if (!t && !r) {
        qsort(ofile, ofile_num, sizeof(char*), file_qsort);
        qsort(dfile, dfile_num, sizeof(char*), file_qsort);
    } else if (!t && r) {
        qsort(ofile, ofile_num, sizeof(char*), r_file_qsort);
        qsort(dfile, dfile_num, sizeof(char*), r_file_qsort);
    } else if (t && !r) {
        qsort(ofile, ofile_num, sizeof(char*), t_file_qsort);
        qsort(dfile, dfile_num, sizeof(char*), t_file_qsort);
    } else {
        qsort(ofile, ofile_num, sizeof(char*), tr_file_qsort);
        qsort(dfile, dfile_num, sizeof(char*), tr_file_qsort);
    }

    // 普通文件选项的执行

    for (int i = 0; i < ofile_num; i++) {
        struct stat statbuf;
        lstat(ofile[i], &statbuf);
        if (I) {
            printf("%7lu ", (unsigned long)statbuf.st_ino);
        }
        if (s) {
            printf("%4lu ", (unsigned long)statbuf.st_blocks / 2);
        }
        if (l) {
            // 类型
            putchar(S_ISREG(statbuf.st_mode) ? '-' : 'd');
            // 权限
            putchar((statbuf.st_mode & S_IRUSR) ? 'r' : '-');
            putchar((statbuf.st_mode & S_IWUSR) ? 'w' : '-');
            putchar((statbuf.st_mode & S_IXUSR) ? 'x' : '-');
            putchar((statbuf.st_mode & S_IRGRP) ? 'r' : '-');
            putchar((statbuf.st_mode & S_IWGRP) ? 'w' : '-');
            putchar((statbuf.st_mode & S_IXGRP) ? 'x' : '-');
            putchar((statbuf.st_mode & S_IROTH) ? 'r' : '-');
            putchar((statbuf.st_mode & S_IWOTH) ? 'w' : '-');
            putchar((statbuf.st_mode & S_IXOTH) ? 'x' : '-');
            // 链接数
            printf(" %2lu ", (unsigned long)statbuf.st_nlink);
            // 用户名
            printf("%s ", getpwuid(statbuf.st_uid)->pw_name);
            // 属组名
            printf("%s ", getgrgid(statbuf.st_gid)->gr_name);
            // 大小
            printf("%6lu ", (unsigned long)statbuf.st_size);
            // mtime
            struct tm* local_time = localtime(&statbuf.st_mtime);
            setlocale(LC_TIME, "zh_CN.UTF-8");
            char time_str[32];
            strftime(time_str, sizeof(time_str), "%m月 %d %H:%M", local_time);
            printf("%s ", time_str);  // 颜色
        }
        printf("%s\n", ofile[i]);
    }

    // 目录文件选项的执行

    for (int i = 0; i < dfile_num; i++) {
        if (dfile_num > 1 || R) {
            printf("%s:\n", dfile[i]);
        }
        struct dirent** d_file;  // 目录下文件的结构体指针数组
        int d_file_num = 0;
        // 目录下文件排序
        if (!t && !r) {
            d_file_num = scandir(dfile[i], &d_file, NULL, scandir_sort);
        } else if (!t && r) {
            d_file_num = scandir(dfile[i], &d_file, NULL, r_scandir_sort);
        } else if (t && !r) {
            d_file_num = scandir(dfile[i], &d_file, NULL, t_scandir_sort);
        } else {
            d_file_num = scandir(dfile[i], &d_file, NULL, tr_scandir_sort);
        }
        if (l || s) {
            // 计算目录下文件的总块数并打印
            blkcnt_t st_blocks = 0;
            for (int j = 0; j < d_file_num; j++) {
                if (!a && d_file[j]->d_name[0] == '.') {
                    continue;
                }
                struct stat d_statbuf;
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", dfile[i],
                         d_file[j]->d_name);
                lstat(full_path, &d_statbuf);
                st_blocks += (d_statbuf.st_blocks / 2);
            }
            printf("总计 ");
            printf("%lu\n", (unsigned long)st_blocks);
        }
        for (int j = 0; j < d_file_num; j++) {
            if (!a && d_file[j]->d_name[0] == '.') {
                continue;
            }
            struct stat d_statbuf;
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", dfile[i],
                     d_file[j]->d_name);
            lstat(full_path, &d_statbuf);
            if (I) {
                printf("%7lu ", (unsigned long)d_statbuf.st_ino);
            }
            if (s) {
                printf("%4lu ", (unsigned long)d_statbuf.st_blocks / 2);
            }
            if (l) {
                putchar(S_ISREG(d_statbuf.st_mode) ? '-' : 'd');
                putchar((d_statbuf.st_mode & S_IRUSR) ? 'r' : '-');
                putchar((d_statbuf.st_mode & S_IWUSR) ? 'w' : '-');
                putchar((d_statbuf.st_mode & S_IXUSR) ? 'x' : '-');
                putchar((d_statbuf.st_mode & S_IRGRP) ? 'r' : '-');
                putchar((d_statbuf.st_mode & S_IWGRP) ? 'w' : '-');
                putchar((d_statbuf.st_mode & S_IXGRP) ? 'x' : '-');
                putchar((d_statbuf.st_mode & S_IROTH) ? 'r' : '-');
                putchar((d_statbuf.st_mode & S_IWOTH) ? 'w' : '-');
                putchar((d_statbuf.st_mode & S_IXOTH) ? 'x' : '-');
                printf(" %2lu ", (unsigned long)d_statbuf.st_nlink);
                printf("%s ", getpwuid(d_statbuf.st_uid)->pw_name);
                printf("%s ", getgrgid(d_statbuf.st_gid)->gr_name);
                printf("%6lu ", (unsigned long)d_statbuf.st_size);
                // mtime
                struct tm* local_time = localtime(&d_statbuf.st_mtime);
                setlocale(LC_TIME, "zh_CN.UTF-8");
                char time_str[32];
                strftime(time_str, sizeof(time_str), "%m月 %d %H:%M",
                         local_time);
                printf("%s ", time_str);
            }
            printf("%s\n", d_file[j]->d_name);  // 颜色
        }
        if (R) {
            for (int j = 2; j < d_file_num; j++) {
                if (!a && d_file[j]->d_name[0] == '.') {
                    continue;
                }
                struct stat d_statbuf;
                char full_path[PATH_MAX];
                if (strcmp(dfile[i], "/")) {
                    snprintf(full_path, sizeof(full_path), "%s/%s", dfile[i],
                             d_file[j]->d_name);
                } else {
                    snprintf(full_path, sizeof(full_path), "%s%s", dfile[i],
                             d_file[j]->d_name);
                }
                lstat(full_path, &d_statbuf);
                if (S_ISDIR(d_statbuf.st_mode)) {
                    ls_print(full_path);
                }
                free(d_file[j]);
            }
        }
        free(d_file);
    }

    free(file);
    free(dfile);
    free(ofile);
    free(parameter);
    return 0;
}