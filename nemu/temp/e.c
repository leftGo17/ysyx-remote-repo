#include <stdio.h>
#include <stdlib.h> // 为了 free()
#include <string.h> // 为了 strcmp()

// Readline 头文件
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    char *line_read = NULL; // 用于存储 readline 返回的指针
    const char *prompt = "请输入内容 (输入 'q' 退出): "; // 定义提示符

    // 使用一个无限循环来持续读取输入
    while (1) {
        // 调用 readline 函数，它会显示提示符并读取用户输入
        // readline 会为读取到的字符串分配内存
        line_read = readline(prompt);

        // 检查 readline 的返回值
        // 如果返回 NULL，表示遇到了 EOF (例如用户按下了 Ctrl+D) 或发生了错误
        if (line_read == NULL) {
            printf("\n读取到文件尾或发生错误，程序退出。\n");
            break; // 退出循环
        }

        // 如果用户输入了内容 (line_read 不是空指针)
        // 检查输入是否为 "q"
        if (strcmp(line_read, "q") == 0) {
            printf("检测到退出指令 'q'，程序结束。\n");
            free(line_read); // 释放为 "q" 分配的内存
            break;           // 退出循环
        }

        // 如果输入的不是 "q"，并且字符串不是空的，则可以处理它
        // （例如，将其添加到历史记录中，然后打印出来）
        if (*line_read) { // 检查字符串是否非空，避免将空行添加到历史
            add_history(line_read); // 将有效的输入行添加到历史记录中，方便用户通过上下箭头键导航
            printf("你输入了: \"%s\"\n", line_read);
            // 在这里可以添加对 line_read 的其他处理逻辑
        }

        // 非常重要：释放由 readline 分配的内存
        // 每次循环都必须释放，否则会导致内存泄漏
        free(line_read);
        line_read = NULL; // 将指针置空，好习惯
    }

    // (可选) 清理 Readline 历史记录等，如果需要的话
    // clear_history(); // 如果你想要在程序退出时清除历史记录

    return 0;
}