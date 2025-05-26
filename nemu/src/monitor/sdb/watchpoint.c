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

#include "sdb.h"

#define NR_WP 32

#define EXPR_MAX_LEN 128 // 监视表达式的最大长度

// 监视点结构体定义
typedef struct watchpoint {
  int NO;                 // 监视点的序号
  struct watchpoint *next; // 指向链表中下一个监视点的指针

  /* TODO: Add more members if necessary */
  char expr[EXPR_MAX_LEN]; // 存储用户输入的表达式字符串
  uint32_t old_val;        // 存储上一次求值时表达式的值
} WP;


static WP wp_pool[NR_WP] = {};
static WP *head = NULL;
static WP *free_ = NULL;

// 初始化监视点池
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i; // 为每个监视点分配一个唯一的序号
    memset(wp_pool[i].expr, 0, sizeof(wp_pool[i].expr));
    wp_pool[i].old_val = 0;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;       // 初始时，没有正在使用的监视点，head 链表为空
  free_ = wp_pool;   // 初始时，所有监视点都在 free_ 链表中
}

WP* new_wp() {
  if (free_ == NULL) {
    Log("错误：没有可用的监视点，最多可设置 %d 个！", NR_WP);
    assert(0); // 立即终止程序
  }
  WP *new_watchpoint = free_; // 取出 free_ 链表的第一个节点
  free_ = free_->next;       // free_ 链表头向前移动，指向下一个空闲节点
  // 将新取出的监视点添加到 head 链表的头部（头插法）
  new_watchpoint->next = head;
  head = new_watchpoint;
  memset(new_watchpoint->expr, 0, sizeof(new_watchpoint->expr));
  new_watchpoint->old_val = 0;
  return new_watchpoint;
}

void free_wp(WP *wp) {
  if (wp == NULL) {
    return; // 避免对空指针进行操作
  }
  // 在 head 链表中找到并移除 wp
  WP *current = head;
  WP *prev = NULL;
  while (current != NULL && current != wp) {
    prev = current;
    current = current->next;
  }
  // 监视点 wp 不在 head 链表中
  if (current == NULL) {
    Log("要释放的监视点 %d 未在活动列表中找到。", wp->NO);
    assert(0); // 如果发生此关键错误则终止程序
  }

  if (prev == NULL) { // wp 是 head 链表的第一个节点
    head = wp->next;
  } else { // wp 在 head 链表的中间或末尾
    prev->next = wp->next;
  }
  // 将 wp 归还到 free_ 链表的头部（头插法）
  wp->next = free_;
  free_ = wp;
  memset(wp->expr, 0, sizeof(wp->expr));
  wp->old_val = 0;
}
void print_wp_info() {
  if (head == NULL) {
    Log("没有设置监视点。\n");
    return;
  }
  printf("编号\t表达式\t\t当前值\n");
  printf("----------------------------------\n");
  WP *current = head;
  while (current != NULL) {
    printf("%d\t%s\t\t0x%08x\n", current->NO, current->expr, current->old_val);
    current = current->next;
  }
}
// 设置一个新的监视点 (sdb 命令：w <expression>)
void sdb_set_watchpoint(char *args) {
  if (args == NULL || strlen(args) == 0) {
    Log("给出表达式\n");
    return;
  }

  WP *new_wp_ptr = new_wp();
  if (new_wp_ptr == NULL) {
    Log("无法分配新的监视点。\n");
    return;
  }
  strncpy(new_wp_ptr->expr, args, EXPR_MAX_LEN - 1);
  new_wp_ptr->expr[EXPR_MAX_LEN - 1] = '\0'; // 确保字符串以 null 结尾
  bool success = true;
  word_t val = expr(new_wp_ptr->expr, &success); // 调用表达式求值函数
  if (!success) {
    Log("表达式'%s'无效\n", new_wp_ptr->expr);
    free_wp(new_wp_ptr); // 释放已分配的监视点
    return;
  }
  new_wp_ptr->old_val = val; // 存储初始值
  printf("监视点 %d 已设置：%s。初始值:0x%08x\n", new_wp_ptr->NO, new_wp_ptr->expr, new_wp_ptr->old_val);
}

// 删除一个监视点 (sdb 命令：d <watchpoint_number>)
void sdb_del_watchpoint(char *args) {
  if (args == NULL || strlen(args) == 0) {
    Log("需要给出表达式的值\n");
    return;
  }
  int wp_no = atoi(args); // 将字符串参数转换为整数（监视点编号）
  WP *current = head;
  WP *target_wp = NULL;

  // 根据编号在 head 链表中查找目标监视点
  while (current != NULL) {
    if (current->NO == wp_no) {
      target_wp = current;
      break;
    }
    current = current->next;
  }
  if (target_wp == NULL) {
    Log("未找到监视点 %d。\n", wp_no);
    return;
  }
  free_wp(target_wp); // 释放找到的监视点
  Log("监视点 %d 已删除。\n", wp_no);
}

// 此函数将在 trace_and_difftest() 中被调用，在每条指令执行后进行检查
bool check_watchpoints() {
  //printf("check");
  WP *current = head;
  bool triggered = false; // 标记是否有监视点被触发

  while (current != NULL) {
    bool success = true;
    word_t new_val = expr(current->expr, &success); // 重新求值表达式

    if (!success) {
      printf("无法计算监视点 %d 的表达式 '%s'。已忽略。\n", current->NO, current->expr);
      current = current->next; // 移动到下一个监视点
      continue;
    }
    if (new_val != current->old_val) {
      printf("\n----------------------------------\n");
      printf("监视点 %d 触发：%s\n", current->NO, current->expr);
      printf("旧值:0x%x\n", current->old_val);
      printf("新值:0x%x\n", new_val);
      printf("程序已暂停。\n");
      printf("----------------------------------\n");

      current->old_val = new_val; // 更新旧值为新值，以便下次比较
      triggered = true;           // 标记已触发监视点
      // 继续检查，防止后面还有变化
    }
    current = current->next;
  }

  return triggered;
}

