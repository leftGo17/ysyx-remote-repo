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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>

enum {
  TK_NOTYPE = 256,
  TK_DEC_INT,         // 十进制整数
  TK_HEX_INT,         // 十六进制整数 (0x开头)
  TK_REG,             // 寄存器 (以$开头)
  TK_EQ,              // ==
  TK_NEQ,             // !=
  TK_AND,             // &&
  TK_LPAREN = '(',
  TK_RPAREN = ')',
  TK_PLUS = '+',
  TK_MINUS = '-',
  TK_MUL = '*',       // 乘法
  TK_DIV = '/',
  TK_DEREF,           // 指针解引用 (与乘法区分)
};
static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},           // 空格串，忽略
  {"0x[0-9a-fA-F]+", TK_HEX_INT}, // 十六进制整数 (0x开头，后跟数字或a-f/A-F)
  {"[0-9]+", TK_DEC_INT},      // 十进制整数
  {"\\$[a-zA-Z0-9]+", TK_REG}, // 寄存器 (以$开头，后跟字母数字)
  {"\\+", TK_PLUS},            // 加号
  {"-", TK_MINUS},             // 减号
  {"\\*", TK_MUL},             // 乘号 / 指针解引用
  {"/", TK_DIV},               // 除号
  {"\\(", TK_LPAREN},          // 左括号
  {"\\)", TK_RPAREN},          // 右括号
  {"==", TK_EQ},               // 相等
  {"!=", TK_NEQ},              // 不相等
  {"&&", TK_AND},              // 逻辑与
};

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
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];//每个token的字符最长32
} Token;
/*__attribute__((used)): 以防止编译器优化掉这个变量，如果编译器认为变量未使用，它可能会为了节省空间而移除它。*/
//接收的最大token为32
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

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

        // 记录 token 信息
        switch (rules[i].token_type) {
          case TK_NOTYPE:
            // 如果是空格串 (TK_NOTYPE)，则忽略，不记录
            break;
          
          default:         
            if (nr_token >= 32) { // 检查 tokens 数组是否已满 (假设tokens数组大小为32)
              Log("tokens数组已满,最多存储32个token.(position %d)\n", position);
              return false; // Token 数组已满，词法分析失败
            }

            tokens[nr_token].type = rules[i].token_type; // 记录 token 类型

            if (substr_len >= sizeof(tokens[nr_token].str)) {
              // 如果子串长度超过 str 成员的容量 (32 - 1 for char + null terminator)
              Log("这个token太长:'%.*s' at position %d is too long (len %d). 截取:to %zu chars.\n",
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
      Log("no match at position %d\n%s\n", position, e);
      return false;
    }
  }

  return true;
}

static int get_op_precedence(int token_type) {
    switch (token_type) {
        // 优先级最低
        case TK_AND:
            return 1;
        // 比较
        case TK_EQ:
        case TK_NEQ:
            return 2;
        // 加减
        case TK_PLUS:
        case TK_MINUS:
            return 3;
        // 乘除、解引用，优先级最高
        case TK_MUL:
        case TK_DIV:
        case TK_DEREF: // 指针解引用是一元运算符，优先级最高
            return 4;
        default:
            return 0; 
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
        if (tokens[p].type == TK_DEC_INT) {
            return strtoul(tokens[p].str, NULL, 10);
        }
        else if (tokens[p].type == TK_HEX_INT) {
            // 十六进制数 (跳过 "0x")
            return strtoul(tokens[p].str + 2, NULL, 16);
        }
        else if (tokens[p].type == TK_REG) {
            // 寄存器
            // 传入寄存器名称，跳过 '$'
            return isa_reg_str2val(tokens[p].str + 1, success_flag);
        }
        else {
            // 单个 token 但不是数值或寄存器，是无效的
            *success_flag = false;
            Log("无效的单个token:at index %d (type: %d, str: %s)\n", p, tokens[p].type, tokens[p].str);
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
                if (tokens[i].type == TK_PLUS || tokens[i].type == TK_MINUS ||
                    tokens[i].type == TK_MUL || tokens[i].type == TK_DIV ||
                    tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ ||
                    tokens[i].type == TK_AND)
                {
                  int current_op_precedence = get_op_precedence(tokens[i].type);
                  if (current_op_precedence > 0) { // It's an operator
                      if (current_op_precedence <= min_precedence) { //<=选择最右边
                          min_precedence = current_op_precedence;
                          main_op_pos = i;
                      }
                  }
                }

            }
        }
        if (paren_balance != 0) { 
            *success_flag = false;
            return 0;
        }
        // 处理一元运算符 (指针解引用)
        // 注意：指针解引用优先级最高，它会作用于其后的子表达式。
        // 所以我们应该在找到主二元运算符之前，或者作为一个特殊情况来处理。
        // 这里，我们把它放在 main_op_pos == -1 的情况下，因为它的优先级比所有二元运算符都高
        // 且它在 eval 的递归调用中，会作为最高优先级的操作符处理。
        if (main_op_pos == -1) { // 没有找到二元主操作符，检查是否为一元解引用
            if (tokens[p].type == TK_DEREF) {
                // 计算解引用操作符后面的子表达式的值
                word_t addr = eval(p + 1, q, success_flag);
                if (!(*success_flag)) return 0;
                return vaddr_read(addr, 4);
            } else {
                *success_flag = false;
                Log("无法确定主操作符\n");
                return 0;
            }
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
            case TK_MINUS: return val1 - val2;
            case TK_MUL:   return val1 * val2;
            case TK_DIV:
                if (val2 == 0) {
                    *success_flag = false;
                    Log("Division by zero\n");
                    return 0;
                }
                return val1 / val2;
            case TK_EQ:    return (val1 == val2);
            case TK_NEQ:   return (val1 != val2);
            case TK_AND:   return (val1 && val2);
            default:
                *success_flag = false;
                Log("无法完成操作 %d\n", op_type);
                return 0;
        }
    }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if (nr_token == 0) {
    *success = false;
    return 0;
  }
    // 区分乘法和指针解引用
  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == TK_MUL) { // 找到一个 '*'
      // 判断是否为指针解引用：
      // 1. 如果 '*' 是第一个 token (i == 0)
      // 2. 如果 '*' 前一个 token 是左括号 '(', 或者其他二元运算符
      if (i == 0 ||
          tokens[i - 1].type == TK_LPAREN ||
          tokens[i - 1].type == TK_PLUS ||
          tokens[i - 1].type == TK_MINUS ||
          tokens[i - 1].type == TK_DIV ||
          tokens[i - 1].type == TK_EQ ||
          tokens[i - 1].type == TK_NEQ ||
          tokens[i - 1].type == TK_AND
          ) {
        tokens[i].type = TK_DEREF; // 标记为解引用
      }
    }
  }

  *success = true; 
  word_t result = eval(0, nr_token - 1, success);

  if (!(*success)) {
    return 0; 
  }
  return result;
}



