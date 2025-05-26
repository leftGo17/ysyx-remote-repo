#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256 // 定义每行最大长度

int main() {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char *str1, *str2;

    // 尝试打开文件
    file = fopen("input", "r"); // 假设文件名为 data.txt，放在程序同目录下
    if (file == NULL) {
        perror("Error opening file");
        return 1; // 打开文件失败，返回错误码
    }

    printf("Reading file line by line and splitting into two strings:\n");
    // 逐行读取文件，直到文件末尾
    while (fgets(line, sizeof(line), file) != NULL) {
        // 移除行末的换行符（如果存在）
        line[strcspn(line, "\n")] = 0;

        // 使用 strtok 分割字符串
        // strtok 会修改原始字符串，所以如果你需要保留原始行，需要先复制
        str1 = strtok(line, " "); // 获取第一个字符串（以空格为分隔符）
        str2 = strtok(NULL, " "); // 获取第二个字符串（从上一个分割点继续）

        if (str1 != NULL && str2 != NULL) {
            printf("String 1: \"%s\", String 2: \"%s\"\n", str1, str2);
        } else if (str1 != NULL) {
            printf("String 1: \"%s\", String 2: (null)\n", str1);
        } else {
            printf("Empty or malformed line.\n");
        }
    }

    // 关闭文件
    fclose(file);

    return 0; // 程序成功执行
}