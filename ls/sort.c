#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int file_sort(const void* a, const void* b) {
    const char* cmp_a = *(const char**)a;
    const char* cmp_b = *(const char**)b;
    int str_a = (cmp_a[0] == '.') ? 1 : 0;
    int str_b = (cmp_b[0] == '.') ? 1 : 0;
    // 字母排序
    while (cmp_a[str_a] && cmp_b[str_b]) {
        int lower_a = tolower((unsigned char)cmp_a[str_a]);
        int lower_b = tolower((unsigned char)cmp_b[str_b]);
        if (lower_a != lower_b) {
            // 字母顺序排序
            return lower_a - lower_b;
        } else {
            // 字母大小写排序
            if (islower(cmp_a[str_a]) && isupper(cmp_b[str_b])) {
                return -1;
            }
            if (isupper(cmp_a[str_a]) && islower(cmp_b[str_b])) {
                return 1;
            }
        }
        str_a++;
        str_b++;
    }
    if (cmp_a[str_a] != cmp_b[str_b]) {
        // str已经到'\0'（短的）排在前面
        return cmp_a[str_a] - cmp_b[str_b];
    }
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

int main() {
    const char* names[] = {".Aa", "A", "a", ".ab", ".B", ".123", "."};
    int n = sizeof(names) / sizeof(names[0]);
    qsort(names, n, sizeof(char*), file_sort);
    printf("排序结果:\n");
    for (int i = 0; i < n; i++) {
        printf("%s\n", names[i]);
    }
    return 0;
}