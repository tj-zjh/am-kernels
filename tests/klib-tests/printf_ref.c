#include <limits.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {
  const int data[] = {
    0, INT_MAX / 17, INT_MAX, INT_MIN, INT_MIN + 1, // 测试零、正数和有符号边界
    UINT_MAX / 17, -1, 1 // UINT_MAX / 17 可由这里的 int 表示；再测 -1 和 1
  };

  for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++) {
    printf("%zu: %d\n", i, data[i]);
  }

  return 0;
}