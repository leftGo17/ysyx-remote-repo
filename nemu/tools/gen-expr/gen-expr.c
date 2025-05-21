/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

// 指向 buf 中当前可以写入的位置
static char *buf_ptr;

#define MAX_DEPTH 5 
static uint32_t choose(uint32_t n) {
  if (n == 0) { // 处理n为0的边界情况，虽然在本程序中不太可能传入0
    return 0;
  }
  return rand() % n; // rand() 生成一个伪随机整数，然后取模n
}

static void gen_char_to_buf(char c) {
  // 检查缓冲区是否还有足够空间存放字符 c 以及末尾的空终止符 '\0'
  if (buf_ptr < buf + sizeof(buf) - 2) { // 至少需要2个字节：1个给c，1个给'\0'
    *buf_ptr = c;       // 将字符 c 放入当前指针位置
    buf_ptr++;          // 移动指针到下一个位置
    *buf_ptr = '\0';    // 在新位置添加空终止符，确保 buf 始终是有效的C字符串
  }
}

static void gen_str_to_buf(const char *s) {
  size_t len = strlen(s); // 获取要追加的字符串的长度
  if ((size_t)(buf_ptr - buf) + len + 1 < sizeof(buf)) {
    strcpy(buf_ptr, s); // 将字符串 s 复制到当前指针位置
    buf_ptr += len;     // 移动指针到追加后字符串的末尾
                        // strcpy 会自动复制 s 的 '\0'，所以 buf_ptr 现在指向这个 '\0'
  }
}

static void gen_num() {
  char num_str[12]; // 缓冲区足以存放一个 uint32_t 转换成的字符串 (最大约10位)
  uint32_t num = choose(100) + 1; // 生成 1 到 100 之间的数字
  sprintf(num_str, "%u", num);    // 将无符号整数格式化为字符串
  gen_str_to_buf(num_str);        // 追加到 buf
}

static void gen_rand_op() {
  char op_char;
  switch (choose(4)) { // 随机选择四种基本运算符中的一种
    case 0: op_char = '+'; break;
    case 1: op_char = '-'; break;
    case 2: op_char = '*'; break;
    default: op_char = '/'; break; // case 3
  }
  //gen_char_to_buf(' ');       // 在运算符前添加一个空格，使表达式更易读
  gen_char_to_buf(op_char);   // 追加选中的运算符字符
  //gen_char_to_buf(' ');       // 在运算符后添加一个空格
}

static void gen_rand_expr_recursive(int depth) {
  if (depth >= MAX_DEPTH) {
    gen_num();
    return;
  }
  int choice = choose(3);
  switch (choice) {
    case 0: // 规则1: <expr> ::= <number>
      gen_num();
      break;
    case 1: // 规则2: <expr> ::= "(" <expr> ")"
      gen_char_to_buf('(');                 // 生成左括号
      gen_rand_expr_recursive(depth + 1); // 递归生成括号内的表达式，深度加1
      gen_char_to_buf(')');                 // 生成右括号
      break;
    default: // 规则3 (choice == 2): <expr> ::= <expr> <op> <expr>
      gen_rand_expr_recursive(depth + 1); // 递归生成左操作数，深度加1
      gen_rand_op();                      // 生成随机运算符
      gen_rand_expr_recursive(depth + 1); // 递归生成右操作数，深度加1
      break;
  }
}

static void gen_rand_expr() {
  buf_ptr = buf;      // 每次生成新的表达式时，重置 buf_ptr 指向 buf 的起始位置
  *buf_ptr = '\0';    // 将 buf 的第一个字符设为 '\0'，即清空 buf 或使其成为一个空字符串
  gen_rand_expr_recursive(0); // 从深度 0 开始调用递归生成函数
}
// static void gen_rand_expr() {
//   buf[0] = '\0';
// }

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
