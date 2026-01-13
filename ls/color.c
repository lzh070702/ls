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

// 颜色宏定义
#define RED "\033[31m"     // 红色
#define GREEN "\033[32m"   // 绿色
#define YELLOW "\033[33m"  // 黄色
#define BLUE "\033[34m"    // 蓝色
#define PURPLE "\033[35m"  // 紫色
#define CYAN "\033[36m"    // 青色
#define RESET "\033[0m"    // 白色

void get_file_color(char* filename, mode_t mode) {
    // 判断文件类型
    if (S_ISDIR(mode)) {
        // 目录
        printf(BLUE "%s\n" RESET, filename);
    } else if (S_ISLNK(mode)) {
        // 符号链接
        printf(CYAN "%s\n" RESET, filename);
    } else if (S_ISCHR(mode) || S_ISBLK(mode) || S_ISFIFO(mode)) {
        // 字符设备或块设备或管道
        printf(YELLOW "%s\n" RESET, filename);
    } else if (S_ISSOCK(mode)) {
        // 套接字
        printf(PURPLE "%s\n" RESET, filename);
    } else if (mode & S_IXUSR) {
        // 可执行文件
        printf(GREEN "%s\n" RESET, filename);
    } else {
        // 根据扩展名判断普通文件
        const char* ext = strrchr(filename, '.');
        if (ext) {
            ext++;  // 跳过点号
            // 压缩文件
            if (strcmp(ext, "gz") == 0 || strcmp(ext, "bz2") == 0 ||
                strcmp(ext, "xz") == 0 || strcmp(ext, "zip") == 0 ||
                strcmp(ext, "tar") == 0 || strcmp(ext, "tgz") == 0 ||
                strcmp(ext, "7z") == 0 || strcmp(ext, "rar") == 0) {
                printf(RED "%s\n" RESET, filename);

            }
            // 图片文件
            else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0 ||
                     strcmp(ext, "png") == 0 || strcmp(ext, "gif") == 0 ||
                     strcmp(ext, "bmp") == 0 || strcmp(ext, "svg") == 0 ||
                     strcmp(ext, "ico") == 0) {
                printf(PURPLE "%s\n" RESET, filename);
            }
            // 媒体文件
            else if (strcmp(ext, "mp3") == 0 || strcmp(ext, "mp4") == 0 ||
                     strcmp(ext, "avi") == 0 || strcmp(ext, "mkv") == 0 ||
                     strcmp(ext, "flac") == 0 || strcmp(ext, "wav") == 0) {
                printf(CYAN "%s\n" RESET, filename);
            }
            // 文档文件
            else if (strcmp(ext, "pdf") == 0 || strcmp(ext, "doc") == 0 ||
                     strcmp(ext, "docx") == 0 || strcmp(ext, "xls") == 0 ||
                     strcmp(ext, "xlsx") == 0 || strcmp(ext, "ppt") == 0 ||
                     strcmp(ext, "pptx") == 0 || strcmp(ext, "txt") == 0) {
                printf(YELLOW "%s\n" RESET, filename);
            }
            // 代码文件
            else if (strcmp(ext, "c") == 0 || strcmp(ext, "cpp") == 0 ||
                     strcmp(ext, "h") == 0 || strcmp(ext, "hpp") == 0 ||
                     strcmp(ext, "py") == 0 || strcmp(ext, "java") == 0 ||
                     strcmp(ext, "js") == 0 || strcmp(ext, "html") == 0 ||
                     strcmp(ext, "css") == 0 || strcmp(ext, "sh") == 0 ||
                     strcmp(ext, "php") == 0) {
                printf(GREEN "%s\n" RESET, filename);
            }
            // 配置文件
            else if (strcmp(ext, "conf") == 0 || strcmp(ext, "cfg") == 0 ||
                     strcmp(ext, "ini") == 0 || strcmp(ext, "json") == 0 ||
                     strcmp(ext, "xml") == 0 || strcmp(ext, "yml") == 0 ||
                     strcmp(ext, "yaml") == 0) {
                printf(CYAN "%s\n" RESET, filename);
            }
        }
    }
    // 普通文件
    printf(RESET "%s\n" RESET, filename);
}

int main() {
    printf(RED "红\n" RESET);
    printf(GREEN "绿\n" RESET);
    printf(BLUE "蓝\n" RESET);
    printf(PURPLE "紫\n" RESET);
    printf(CYAN "青\n" RESET);
    printf(RESET "无\n" RESET);
    return 0;
}
