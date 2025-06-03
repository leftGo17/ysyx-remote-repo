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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>
#include <macro.h>


static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");
  /*line_read所指向的字符串不为空*/
  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args) {
  if (args == NULL){
    cpu_exec(1);
    return 0;
  }
  else{
    cpu_exec(atoi(args));
    return 0;
  }
}

static int cmd_info(char * args){
  if (args == NULL){
    printf("Please enter \"info r\"\n");
    return 0;
  }
  if (strcmp(args, "r") == 0){
    isa_reg_display();
    return 0;
  }
  else if (strcmp(args, "w") == 0){
    print_wp_info();
    return 0;
  }
  else
    return 0;
}

static int cmd_x(char * args){
  char *arg1 = strtok(NULL, " ");  
  char *arg2 = strtok(NULL, " ");
  if (arg1 == NULL || arg2 == NULL)
  {
    Log("Please enter \"x N EXPR\"\n");
    return 0;
  }
  int n = atoi(arg1);
  bool success;
  word_t ans;
  ans = expr(arg2, &success);
  if (success)
  {
    printf("address-expr=0x%08x\n", ans);
    vaddr_t addr = ans;
    int i;
    for (i = 0; i < n; i+=1)
    {
      int len = 4;
      word_t data = vaddr_read(addr, len);
      printf("addr=0x%08x; data=0x%08x\n", addr, data);
      addr += 4;
    }
  }
  else Log("Error expr\n");

  return 0;
}

static int cmd_p(char * args){
  bool success;
  word_t ans;
  ans = expr(args, &success);
  if (success)
  {
    printf("DEC:expr=%u\n", ans); 
    printf("HEX:expr=0x%08x\n", ans);
  }
  else Log("error_expr\n");
  return 0;
}

static int cmd_w(char * args){
  sdb_set_watchpoint(args);
  return 0;
}

static int cmd_d(char * args){
  sdb_del_watchpoint(args);
  return 0;
}

void test_expr();
static int cmd_test(char * args){
  test_expr();
  return 0;
}

static int cmd_help(char *args);

void test_expr(){
  //printf("try test_expr!\n");
  FILE *filee;
  char line[256];
  char *str1, *str2;

  filee = fopen("/home/zjp/code/ysyx-workbench/nemu/src/monitor/sdb/data_input", "r"); 
  if (filee == NULL) {
      Log("open failed!\n");
      perror("Error opening file");
  }
  //if (filee != NULL)printf("test!!\n");

  //char *test=fgets(line, sizeof(line), filee);
 //printf("%s\n",test);
  while (fgets(line, sizeof(line), filee) != NULL) {
      line[strcspn(line, "\n")] = 0;
      //printf("try find different!\n");
      str1 = strtok(line, " "); 
      str2 = strtok(NULL, " "); 
      bool success;
      word_t ans;
      ans = expr(str2, &success);
      if (success)printf("expr=%u %s\n", ans, str1);
      else printf("error_expr!!\n");
  }
  //printf("test!!\n");
  fclose(filee);
}

/*int (*handler) (char *); 定义了一个名为 handler 的变量，这个变量是一个指针，它可以指向任何满足以下条件的函数：
接受一个 char * 类型的参数。
返回一个 int 类型的值*/
//cmd列表第一个参数是命令的名字，第二个是描述，第三个是函数指针
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "si [N]: Continue the execution by N steps", cmd_si},
  { "info", "info r: Print the data of register(or watchpool)", cmd_info},
  { "x", "x N EXPR: Print N types from the begin of address  EXPR", cmd_x},
  { "p", "p EXPR: Caculate the date of EXPR", cmd_p},
  { "w", "w EXPR: Set watchpools and stop when the data change", cmd_w},
  { "d", "d N: Delete N-th watchpool", cmd_d},
  { "test", "test expr", cmd_test}
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

//这里再使用strtok，就要把第一个参数赋值成为NULL，继续上一次的寻找
static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    //没有参数就打印cmd_table表
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    Log("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  MUXDEF(__BATCH_MODE__ ,sdb_set_batch_mode(),);
  //test_expr();
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
  //rl_gets()获得屏幕读入的一行字符串
  for (char *str; (str = rl_gets()) != NULL; ) {
    //*str_end = '/0';
    //str_end是字符串'/0'所在位置的地址
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    //读取的第一个字符作为指令
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    //后面的作为参数字符串
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    //strcmp函数用来比较两个字符串，如果完全相同就返回0,也就是匹配到了gdb命令
    //找到对应的字符串就跳出循环，继续下一次即可
    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }
    if (i == NR_CMD) { Log("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  //正则表达式
  init_regex();

  /* Initialize the watchpoint pool. */
  //Watchpoint Pool，监视点池
  init_wp_pool();
}
