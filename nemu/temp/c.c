#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h> // For uint32_t
#include <stdlib.h> // For strtoul
#include <limits.h> // For UINT32_MAX

// Assume Token structure and tokens array are defined and populated as per problem context
typedef struct token {
  int type;
  char str[32];
} Token;

// These would be external or global, populated by make_token
extern Token tokens[256]; // Assuming a maximum of 256 tokens for an expression
extern int nr_token;

// Token types enum (ensure this matches your tokenizer's definitions)
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
  // ... any other token types your tokenizer might produce
};

typedef uint32_t word_t; // As per "所有结果都是uint32_t类型"

// Forward declaration for the recursive evaluation function
static word_t eval(int p, int q, bool *success);

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
            uint32_t val_ul = strtoul(tokens[p].str, NULL, 10);
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
 v 
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if (nr_token == 0) { // Empty expression or only whitespace tokens
    *success = false;
    return 0;
  }

  *success = true; // Initialize success to true before evaluation
  word_t result = eval(0, nr_token - 1, success);

  if (!(*success)) {
    // An error occurred during evaluation.
    // `eval` or its sub-calls would have set `*success` to false.
    return 0; // Return 0 on failure as a convention
  }

  return result;
}


#include <stdio.h>    // 主要用于调试时的 printf，实际提交时可能不需要
#include <string.h>   // 用于 strncpy 等（如果 Tokenizer 需要）
#include <stdbool.h>  // 用于 bool 类型
#include <stdint.h>   // 用于 uint32_t (word_t)
#include <stdlib.h>   // 用于 strtoul (字符串转无符号长整型)
#include <limits.h>   // 用于 UINT32_MAX (uint32_t 的最大值)
// #include <assert.h>   // 如果需要使用 assert(0)

// 假设 Token 结构体、全局 tokens 数组和 nr_token 已按题目上下文定义和填充
typedef struct token {
  int type;       // Token 的类型，例如 TK_NUMBER, '+', '(', 等
  char str[32];   // Token 的字符串表示，主要用于数字
} Token;

// 这些变量由 make_token 函数填充，这里假设它们是外部或全局的
// 为了独立编译和理解，这里用 extern 声明，实际项目中可能直接定义或通过其他方式访问
extern Token tokens[256]; // 假设表达式最多包含 256 个 token
extern int nr_token;      // 实际由 make_token 产生的 token 数量

// Token 类型枚举 (请确保这里的定义与你的词法分析器 make_token 一致)
enum {
  TK_NOTYPE = 256, // 一个示例值，确保不与 ASCII 字符冲突
  TK_DEC_INT,      // 十进制整数类型
  TK_NUMBER = TK_DEC_INT, // 为 TK_DEC_INT 设置一个更通用的别名 TK_NUMBER
  TK_LPAREN = '(', // 左括号，可以直接使用其 ASCII 值
  TK_RPAREN = ')', // 右括号
  TK_PLUS = '+',   // 加号
  TK_MINUS = '-',  // 减号
  TK_MUL = '*',    // 乘号
  TK_DIV = '/',    // 除号
  // 如果你的词法分析器还产生其他类型的 token (例如 TK_EQ 表示 '==')，也应在此定义
};

// 根据题目要求 "所有结果都是uint32_t类型"，定义 word_t
typedef uint32_t word_t;

// 递归求值函数的前向声明
static word_t eval(int p, int q, bool *success_flag);

// 获取操作符优先级的辅助函数
static int get_op_precedence(int token_type) {
    switch (token_type) {
        case TK_PLUS:
        case TK_MINUS:
            return 1; // '+' 和 '-' 的优先级较低
        case TK_MUL:
        case TK_DIV:
            return 2; // '*' 和 '/' 的优先级较高
        default:
            return 0; // 表示不是一个我们关心的参与优先级比较的操作符（例如括号或数字）
    }
}

// 检查 tokens[p...q] 是否被一对匹配的括号完全包围，并且内部括号也平衡。
// 如果发现括号不匹配等语法错误，会设置 *success_flag = false。
static bool check_parentheses_is_surrounded_and_valid(int p, int q, bool *success_flag) {
    // 1. 检查首尾是否是括号
    if (tokens[p].type != TK_LPAREN || tokens[q].type != TK_RPAREN) {
        return false; // 不是以 '(' 开头并以 ')' 结尾
    }

    // 2. 检查从 p 到 q 的整个范围内的括号是否平衡，
    //    并且确保 tokens[p] 和 tokens[q] 是包围整个子表达式的唯一最外层括号对。
    int balance = 0;
    for (int i = p; i <= q; ++i) {
        if (tokens[i].type == TK_LPAREN) {
            balance++;
        } else if (tokens[i].type == TK_RPAREN) {
            balance--;
        }

        // 如果 balance 小于 0，说明右括号多于左括号，例如 "())" 或在表达式中间出现 ")("
        if (balance < 0) {
            // printf("调试：括号不匹配 - 在 token %d 处右括号过多。\n", i);
            *success_flag = false;
            return false;
        }
        
        // 关键检查：如果 tokens[p] 是 '('，并且在到达 tokens[q] 之前 balance 已经变回 0，
        // 这意味着 tokens[p] 的匹配右括号在 tokens[q] 之前就出现了。
        // 例如，对于 "(A) + B" 这样的表达式，如果 p 指向第一个 '('，q 指向 B 的末尾，
        // 那么当扫描到 A 后面的 ')' 时，balance 会变为 0，但此时还没有到达 q。
        // 这表明 tokens[p] 和 tokens[q] 并不是包围整个表达式 p...q 的那一对括号。
        if (i < q && balance == 0 && tokens[p].type == TK_LPAREN) {
             // printf("调试：在 token %d 处，对应于 token %d 的左括号已闭合，但未到达 token %d。\n", i, p, q);
             return false;
        }
    }

    // 3. 最终检查：如果扫描完整个 p...q 范围后，balance 不为 0，则括号不平衡，例如 "(("
    if (balance != 0) {
        // printf("调试：括号不平衡 - 扫描完 tokens[%d...%d] 后，最终 balance 为 %d。\n", p, q, balance);
        *success_flag = false;
        return false;
    }
    
    // 如果所有检查都通过，说明 tokens[p...q] 的形式是 "(A)"，且 A 本身括号平衡
    return true;
}

// 核心递归求值函数
static word_t eval(int p, int q, bool *success_flag) {
    // 如果在之前的递归调用中已经检测到错误，则直接返回，不再继续计算
    if (!(*success_flag)) {
        return 0;
    }

    // 基本情况1：无效的表达式范围
    if (p > q) {
        // 通常这种情况是因为操作符缺少操作数，例如 "1 +" 这种表达式的右侧，
        // 或者在尝试分割表达式 "op val" 时，op 左侧的子表达式范围会是 p > q。
        // printf("调试：无效的表达式范围 p=%d, q=%d\n", p, q);
        *success_flag = false;
        return 0;
    }
    // 基本情况2：单个 token
    else if (p == q) {
        // 此时子表达式只有一个 token，它必须是一个数字
        if (tokens[p].type == TK_NUMBER) {
            char *endptr;
            // 使用 strtoul 将数字字符串转换为 unsigned long
            // 题目要求结果为 uint32_t，且通常表达式中的数字是非负的
            unsigned long val_ul = strtoul(tokens[p].str, &endptr, 10); // 10 表示十进制

            // 检查转换是否成功：endptr 应指向字符串末尾的 '\0'
            // 并且 tokens[p].str 不应等于 endptr (意味着没有转换任何数字)
            if (*endptr != '\0' || tokens[p].str == endptr) {
                // printf("调试：无效的数字格式 '%s' (token %d)\n", tokens[p].str, p);
                *success_flag = false;
                return 0;
            }
            // 检查是否溢出 uint32_t 的范围
            if (val_ul > UINT32_MAX) {
                // printf("调试：数字 '%s' 溢出 uint32_t (token %d)\n", tokens[p].str, p);
                *success_flag = false;
                return 0;
            }
            return (word_t)val_ul; // 转换成功，返回数值
        } else {
            // 单个 token 但不是数字（例如单独的 '+' 或 '('），这是语法错误
            // printf("调试：期望单个 token 为数字，但得到类型 %d ('%s') (token %d)\n", tokens[p].type, tokens[p].str, p);
            *success_flag = false;
            return 0;
        }
    }
    // 情况3：检查表达式是否被一对匹配的括号完全包围，形如 "(A)"
    else if (check_parentheses_is_surrounded_and_valid(p, q, success_flag)) {
        // 如果 check_parentheses_is_surrounded_and_valid 内部检测到错误，*success_flag 会被设为 false
        if (!(*success_flag)) {
            return 0;
        }
        // 如果是 "(A)" 的形式，则剥去括号，对内部的 A (即 tokens[p+1...q-1]) 进行递归求值
        return eval(p + 1, q - 1, success_flag);
    }
    // 情况4：处理其他形式的表达式（通常是需要寻找主操作符的，如 "A + B"）
    else {
        // 如果 check_parentheses_is_surrounded_and_valid 返回 false 但 *success_flag 仍为 true，
        // 说明表达式不是 "(A)" 的形式，但到目前为止其括号结构可能是合法的（例如 "A+B" 或 "(A)+B"）。
        // 但如果 *success_flag 已经被 check_parentheses_is_surrounded_and_valid 设为 false（例如对于 "())" 这样的结构），
        // 那么这里应该直接返回。
        if (!(*success_flag)) {
            return 0;
        }

        int main_op_pos = -1;      // 主操作符在 tokens 数组中的位置
        int min_precedence = 100;  // 用一个较大值初始化最低优先级，确保任何实际操作符优先级都比它小
        int paren_balance = 0;     // 用于跟踪当前扫描位置的括号层级

        // 从左到右扫描 tokens[p...q] 来寻找主操作符
        // 主操作符特点：1. 不在括号内 2. 优先级最低 3. 同等最低优先级时，取最右边的
        for (int i = p; i <= q; ++i) {
            if (tokens[i].type == TK_LPAREN) {
                paren_balance++;
            } else if (tokens[i].type == TK_RPAREN) {
                paren_balance--;
                if (paren_balance < 0) { // 在扫描过程中发现括号不匹配
                    // printf("调试：在寻找主操作符时发现括号不匹配 (token %d)\n", i);
                    *success_flag = false;
                    return 0;
                }
            } else if (paren_balance == 0) { // 当前 token 不在任何括号内部 (处于最外层)
                int current_op_precedence = get_op_precedence(tokens[i].type);
                if (current_op_precedence > 0) { // 如果当前 token 是一个我们关心的操作符
                    // 如果当前操作符的优先级小于或等于目前找到的最低优先级，
                    // 更新主操作符为当前操作符。使用 "<=" 可以确保当优先级相同时，选的是最右边的那个。
                    if (current_op_precedence <= min_precedence) {
                        min_precedence = current_op_precedence;
                        main_op_pos = i;
                    }
                }
            }
        }
        
        // 扫描结束后，检查整个 p...q 区间的括号是否平衡
        if (paren_balance != 0) {
            // printf("调试：在寻找主操作符后，发现 tokens[%d...%d] 区间括号不平衡，最终 balance 为 %d\n", p, q, paren_balance);
            *success_flag = false;
            return 0;
        }

        // 如果没有找到主操作符 (main_op_pos 仍为 -1)
        // 这意味着表达式结构有问题，例如 "1 2" (两个数字相邻) 或 "(1) (2)"
        // (p==q 的情况已处理，被括号包围的情况也已处理)
        if (main_op_pos == -1) {
            // printf("调试：在 tokens[%d...%d] 区间未找到主操作符或表达式格式错误。\n", p, q);
            *success_flag = false;
            return 0;
        }
        
        // 找到了主操作符，在其两侧递归求值
        word_t val1 = eval(p, main_op_pos - 1, success_flag);
        // 如果左侧求值失败，则直接返回错误
        if (!(*success_flag)) return 0;

        word_t val2 = eval(main_op_pos + 1, q, success_flag);
        // 如果右侧求值失败，则直接返回错误
        if (!(*success_flag)) return 0;

        // 根据主操作符的类型执行运算
        int op_type = tokens[main_op_pos].type;
        switch (op_type) {
            case TK_PLUS:  return val1 + val2;
            case TK_MINUS: return val1 - val2; // uint32_t 的减法是回绕的
            case TK_MUL:   return val1 * val2;
            case TK_DIV:
                if (val2 == 0) { // 检查除零错误
                    // printf("调试：发生除零错误 (token %d)\n", main_op_pos);
                    *success_flag = false;
                    return 0;
                }
                return val1 / val2; // uint32_t 的除法是整数除法
            default:
                // 理论上不应该到达这里，因为主操作符应该是已知的类型
                // printf("调试：未知的主操作符类型 %d (token %d)\n", op_type, main_op_pos);
                *success_flag = false; 
                // 伪代码中是 assert(0)，这里我们选择设置错误标志
                return 0;
        }
    }
}

// 用户需要完成的主函数 expr
word_t expr(char *e, bool *success) {
  // 1. 调用词法分析器 make_token (假设它已在别处定义并能正确工作)
  // make_token 会将结果存储在全局的 tokens 数组中，并设置 nr_token
  if (!make_token(e)) {
    *success = false; // 词法分析失败
    return 0;
  }

  // 2. 检查是否有有效的 token 产生
  if (nr_token == 0) {
    // 空表达式或者只包含空格（如果空格被词法分析器忽略且不产生 TK_NOTYPE token）
    *success = false;
    return 0;
  }

  // 3. 初始化 success 标志为 true，然后调用 eval 进行求值
  *success = true;
  word_t result = eval(0, nr_token - 1, success); // 对整个 token 序列求值

  // 4. 检查求值过程中是否发生错误
  if (!(*success)) {
    // 如果 eval 或其子调用中发生错误，*success 会被设为 false
    return 0; // 按惯例，在失败时返回 0 (具体返回值可能取决于项目要求)
  }

  return result; // 求值成功，返回结果
}