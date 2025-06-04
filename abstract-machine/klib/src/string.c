#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t count = 0;
  while (s[count] != '\0') {
    count++;
  }
  return count;
}

char *strcpy(char *dst, const char *src) {
  char *ret = dst;
  while ((*dst++ = *src++) != '\0');
  return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *ret = dst;
  size_t i;
  // 最多从 src 复制 n 个字符到 dst
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  // 如果 src 的长度小于 n，则用空字符填充 dst 的剩余部分 (直到n个字符)
  for ( ; i < n; i++) {
    dst[i] = '\0';
  }
  return ret;
}

char *strcat(char *dst, const char *src) {
  char *ret = dst;
  // 将 dst 指针移动到 dst 中现有字符串的末尾
  while (*dst != '\0') {
    dst++;
  }
  // 将 src 复制到 dst 的末尾，包括空终止符
  while ((*dst++ = *src++) != '\0');
  return ret;
}

int strcmp(const char *s1, const char *s2) {
  // 将字符指针转换为 unsigned char 指针进行比较，以确保正确的字符比较行为
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  // 循环直到找到不匹配的字符或到达其中一个字符串的末尾
  while (*p1 != '\0' && *p1 == *p2) {
    p1++;
    p2++;
  }
  // 返回第一个不匹配字符的差值，或者如果一个字符串是另一个的前缀，则返回非零值
  return *p1 - *p2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  if (n == 0) {
    return 0; // 如果 n 为 0，则不比较，返回 0
  }
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  // 最多比较 n 个字符
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2; // 如果字符不匹配，返回差值
    }
    if (*p1 == '\0') { // 如果到达字符串末尾 (且字符匹配)，则认为相等
      return 0;
    }
    p1++;
    p2++;
  }
  return 0; // 如果比较了 n 个字符且都匹配，则返回 0
}

void *memset(void *s, int c, size_t n) {
  unsigned char *ptr = (unsigned char *)s;
  unsigned char val = (unsigned char)c; // 将 int c 转换为 unsigned char
  for (size_t i = 0; i < n; i++) {
    ptr[i] = val; // 将内存区域的每个字节设置为 val
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *d_ptr = (unsigned char *)dst;
  const unsigned char *s_ptr = (const unsigned char *)src;

  if (d_ptr == s_ptr || n == 0) {
    return dst; // 如果源和目标相同或大小为0，则无需操作
  }

  // 检查内存区域是否重叠并确定复制方向
  // 如果目标区域在源区域之后并且它们有重叠，则从后向前复制
  if (d_ptr > s_ptr && d_ptr < s_ptr + n) {
    // 从后向前复制
    for (size_t i = n; i > 0; i--) {
      d_ptr[i - 1] = s_ptr[i - 1];
    }
  } else {
    // 从前向后复制 (没有重叠，或者目标区域在源区域之前，或者目标区域完全在源区域之后)
    for (size_t i = 0; i < n; i++) {
      d_ptr[i] = s_ptr[i];
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  // memcpy 的行为在内存区域重叠时是未定义的，因此简单的从前向后复制即可
  // 题目要求假设内存区域不重叠
  unsigned char *d_ptr = (unsigned char *)out;
  const unsigned char *s_ptr = (const unsigned char *)in;
  for (size_t i = 0; i < n; i++) {
    d_ptr[i] = s_ptr[i];
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  if (n == 0) {
    return 0; // 如果 n 为 0，则不比较，返回 0
  }
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  // 逐字节比较最多 n 个字节
  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] - p2[i]; // 如果字节不匹配，返回差值
    }
  }
  return 0; // 如果比较了 n 个字节且都匹配，则返回 0
}

#endif
