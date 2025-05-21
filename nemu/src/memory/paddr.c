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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>
//PG_ALIGN: 这是一个宏，展开后是 __attribute((aligned(4096)))。
//__attribute((aligned(4096))): 这是 GCC (以及 Clang 等兼容编译器) 的一个扩展特性，用于指定变量的最小对齐要求。
//这里，它确保 pmem 数组的起始地址将是 4096 字节的倍数。4096 字节通常是操作系统内存页（Page）的大小，所以 pmem 是页对齐的。
//= {}: 这是聚合初始化。对于静态存储持续时间的数组，如果提供了初始化列表（即使是空的 {}），所有未显式初始化的元素都会被零初始化。
//因此，在程序开始执行时，pmem 数组的所有字节最初都被设置为 0。
#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

//pmem_read从pmem真实的读取数据，首先就要获得这个数据真正的地址，也就是guest_to_host
//长度始终为4，也就是32位，对应32位机器
//也就是取出4字节的数
static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
//分配指定“字节”大小，通过aeeset保证内存分配成功
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
// 一个名为 pmem 的 128 MiB 大小的静态字节数组被创建。它的起始地址是 4096 字节对齐的，并且其所有 134,217,728 个字节都被初始化为 0。
  IFDEF(CONFIG_MEM_RANDOM, memset(pmem, rand(), CONFIG_MSIZE));
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

//likely分支预测，因为这个表达式大概率满足
//word_t是32位 uint32_t
word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
