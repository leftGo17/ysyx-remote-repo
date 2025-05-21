#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <regex.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h> // 为了 free()
#include <string.h> // 为了 strcmp()

#define word_t uint32_t
enum {
  TK_NOTYPE = 256, // Example, ensure it doesn't clash with char values
  TK_DEC_INT,
  TK_NUMBER = TK_DEC_INT, // Alias
  TK_LPAREN = '(',
  TK_RPAREN = ')',
  TK_PLUS = '+',
  TK_MINUS = '-',
  TK_MUL = '*',
  TK_DIV = '/',
  TK_EQ
};
static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},         // 空格串 (一个或多个空格), 类型为 TK_NOTYPE, 会被忽略
  {"[0-9]+", TK_DEC_INT},    // 十进制整数 (一个或多个数字)
  {"\\+", TK_PLUS},              // 加号 ( '+' 是元字符，需转义)
  {"-", TK_MINUS},                // 减号 (普通字符，无需转义)
  {"\\*", TK_MUL},              // 乘号 ( '*' 是元字符，需转义),第一个反斜杠，因为c本身\有特殊意义，而使用\\相当于使用了反斜杠
  {"/", TK_DIV},                // 除号 (普通字符，无需转义)
  {"\\(", TK_LPAREN},              // 左括号 ( '(' 是元字符，需转义)
  {"\\)", TK_RPAREN},              // 右括号 ( ')' 是元字符，需转义)
  {"==", TK_EQ},             // 双等号 
};

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      assert(0);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        printf("match rules[%d] = \"%s\" at position %d with len %d: %.*s\n",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        // 记录 token 信息
        switch (rules[i].token_type) {
          case TK_NOTYPE:
            // 如果是空格串 (TK_NOTYPE)，则忽略，不记录
            break;

          case TK_DEC_INT: // 十进制整数
          default:
            if (nr_token >= 32) { // 检查 tokens 数组是否已满 (假设tokens数组大小为32)
              printf("Error: Too many tokens. Token array is full. (at position %d)\n", position);
              return false; // Token 数组已满，词法分析失败
            }

            tokens[nr_token].type = rules[i].token_type; // 记录 token 类型

            // 记录 token 的字符串形式
            // 对于数字，这是必需的。对于运算符和括号，记录下来也有助于调试。
            if (substr_len >= sizeof(tokens[nr_token].str)) {
              // 如果子串长度超过 str 成员的容量 (32 - 1 for char + null terminator)
              printf("Warning: Token '%.*s' at position %d is too long (len %d). Truncating to %zu chars.\n",
                     substr_len, substr_start, position, substr_len, sizeof(tokens[nr_token].str) - 1);
              strncpy(tokens[nr_token].str, substr_start, sizeof(tokens[nr_token].str) - 1);
              tokens[nr_token].str[sizeof(tokens[nr_token].str) - 1] = '\0'; // 确保空字符结尾
            } else {
              strncpy(tokens[nr_token].str, substr_start, substr_len);
              tokens[nr_token].str[substr_len] = '\0'; // 确保空字符结尾
            }
            
            nr_token++; // 已识别的 token 数量加一
            break;
        }
        position += substr_len;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

// Function to get operator precedence
static int get_op_precedence(int token_type) {
    switch (token_type) {
        case TK_PLUS:
        case TK_MINUS:
            return 1; // 低优先级
        case TK_MUL:
        case TK_DIV:
            return 2; // 高优先级
        default:
            return 0; // 不是合法运算法
    }
}

static bool check_parentheses_is_surrounded_and_valid(int p, int q, bool *success_flag) {
    if (tokens[p].type != TK_LPAREN || tokens[q].type != TK_RPAREN) {
        return false; 
    }
    int balance = 0;
    for (int i = p; i <= q; ++i) {
        if (tokens[i].type == TK_LPAREN) {
            balance++;
        } else if (tokens[i].type == TK_RPAREN) {
            balance--;
        }
        if (balance < 0) { // e.g., ( ) ) or ) (
            *success_flag = false; 
            return false;
        }
        //如果出现(exp1) + exp2这种情况，说明（）里面不是最小的exp
        if (i < q && balance == 0) { 
             return false;
        }
    }
    if (balance != 0) { // e.g. ( ( )
        *success_flag = false;
        return false;
    }
    return true;
}


static word_t eval(int p, int q, bool *success_flag) {
    if (!(*success_flag)) { return 0;}

    if (p > q) {
        *success_flag = false;
        return 0;
    } 
    else if (p == q) {
        if (tokens[p].type == TK_NUMBER) {
            word_t val_ul = strtoul(tokens[p].str, NULL, 10);
            return val_ul;
        } 
        else {
            *success_flag = false;
            return 0;
        }
    } 
    else if (check_parentheses_is_surrounded_and_valid(p, q, success_flag)) {
        if (!(*success_flag)) {return 0;}
        return eval(p + 1, q - 1, success_flag);
    } 
    else {
        if (!(*success_flag)) {return 0;}

        int main_op_pos = -1;//找到的主操作符位置
        int min_precedence = 100; //优先级处理，100为最低优先级
        int paren_balance = 0;//在多少层的括号里

        // 1 不在括号里 2 优先级最低 3 同等优先级，取最右边
        for (int i = p; i <= q; ++i) {
            if (tokens[i].type == TK_LPAREN) {
                paren_balance++;
            } 
            else if (tokens[i].type == TK_RPAREN) {
                paren_balance--;
                if (paren_balance < 0) { 
                    *success_flag = false;
                    return 0;
                }
            } 
            else if (paren_balance == 0) { //表示不在括号里面
                int current_op_precedence = get_op_precedence(tokens[i].type);
                if (current_op_precedence > 0) { // It's an operator
                    if (current_op_precedence <= min_precedence) { //<=选择最右边
                        min_precedence = current_op_precedence;
                        main_op_pos = i;
                    }
                }
            }
        }
        if (paren_balance != 0) { 
            *success_flag = false;
            return 0;
        }
        if (main_op_pos == -1) {
            *success_flag = false;
            return 0;
        }
        word_t val1 = eval(p, main_op_pos - 1, success_flag);
        if (!(*success_flag)) return 0;

        word_t val2 = eval(main_op_pos + 1, q, success_flag);
        if (!(*success_flag)) return 0; 

        int op_type = tokens[main_op_pos].type;
        switch (op_type) {
            case TK_PLUS:  return val1 + val2;
            case TK_MINUS: return val1 - val2; // uint32_t subtraction wraps around
            case TK_MUL:   return val1 * val2;
            case TK_DIV:
                if (val2 == 0) {
                    *success_flag = false;
                    return 0;
                }
                return val1 / val2;
            default:
                *success_flag = false; 
                return 0;
        }
    }
}

word_t expr(char *e, bool *success) {
  printf("!!\n");  
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  int i = 0;
  for (i = 0; i < nr_token; i++)printf("%i:%d:%s\n",i,tokens[i].type,tokens[i].str);
  if (nr_token == 0) {
    *success = false;
    return 0;
  }

  *success = true; 
  word_t result = eval(0, nr_token - 1, success);

  if (!(*success)) {
    return 0; 
  }
  return result;
}

int main()
{
    char *line_read = NULL; 
    //char *prompt = "请输入内容 (输入 'q' 退出): "; 
    while (1) {
        line_read = readline("(nemu) ");
        if (line_read == NULL) {
            printf("\n读取到文件尾或发生错误程序退出。\n");
            break; // 退出循环
        }
        if (strcmp(line_read, "q") == 0) {
            printf("检测到退出指令 'q'，程序结束。\n");
            free(line_read); // 释放为 "q" 分配的内存
            break;           // 退出循环
        }
        char *str = line_read;
        bool success = true;
        init_regex();
        word_t ans = expr(str, &success);
        if (success)printf("%u\n",ans);
        else printf("error\n");

        free(line_read);
        line_read = NULL; // 将指针置空，好习惯
    }
    return 0;
}

