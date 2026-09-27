#include <klib.h>
#include <limits.h> 

#define N 32 // 测试数组的字节数
static unsigned char data[N]; // 被 memset 写入并由测试代码检查的数组

#define SRC_BASE 0x80 // 源数组的起始测试值
static unsigned char src[N]; // memcpy 的独立源数组

static void reset_data(void) { // 每轮测试前恢复数组的初始内容
  for (int i = 0; i < N; i++) {
    data[i] = (unsigned char)(i + 1); // 让每个位置都有不同的初始值
  }
}

static void reset_src(void) { // 初始化 memcpy 的源数组
  for (int i = 0; i < N; i++) {
    src[i] = (unsigned char)(SRC_BASE + i);
  } 
}

static void check_src(void) { // 检查源数组没有被 memcpy 修改
  for (int i = 0; i < N; i++) {
    assert(src[i] == (unsigned char)(SRC_BASE + i));
  }
}

static void check_seq(int l, int r, int first) { // 检查 [l, r) 是否为递增序列
  for (int i = l; i < r; i++) {
    assert(data[i] == first + (i - l));
  }
}

static void check_eq(int l, int r, unsigned char expected) { // 检查 [l, r) 是否都等于 expected
  for (int i = l; i < r; i++) {
    assert(data[i] == expected);
  }
}

static void test_memset(void) { // 测试不同位置和长度的 memset 写入
  for (int l = 0; l < N; l++) { // 依次选择写入区间的左边界
    for (int r = l; r <= N; r++) { // 依次选择右边界，r == l 时长度为 0
      reset_data(); // 每个用例都从相同的初始数组开始
      int value = (l + r) / 2; // 为当前用例选择填充值
      void *ret = memset(data + l, value, (size_t)(r - l)); // 将 [l, r) 填成 value
      assert(ret == data + l); // memset 应返回传入的起始地址
      check_seq(0, l, 1); // 检查写入区间左边没有被改动
      check_eq(l, r, (unsigned char)value); // 检查写入区间中的字节都正确
      check_seq(r, N, r + 1); // 检查写入区间右边没有被改动
    }
  }
}

static void test_memcpy(void) { // 测试不同长度和位置的 memcpy
  reset_src();

  for (int l = 0; l < N; l++) {
    for (int r = l; r <= N; r++) {
      int n = r - l;

      for (int s = 0; s < N && s + n <= N; s++) { // 选择一个合法的源区间
        reset_data(); // 恢复目标数组，保证每个用例从相同状态开始

        void *ret = memcpy(data + l, src + s, (size_t)n);
        assert(ret == data + l); // memcpy 应返回目标区间的起始地址

        check_seq(0, l, 1);
        for (int i = 0; i < n; i++) {
          assert(data[l + i] == (unsigned char)(SRC_BASE + s + i));
        }
        check_seq(r, N, r + 1);

        check_src();
      }
    }
  }
} 

static void test_memcmp(void) { // 测试 memcmp 对相等和不同字节的比较结果
  unsigned char left[N];
  unsigned char right[N];

  for (int i = 0; i < N; i++) {
    left[i] = (unsigned char)(i + 1);
    right[i] = left[i];
  }

  for (size_t n = 0; n <= (size_t)N; n++) {
    assert(memcmp(left, right, n) == 0);
  }

  for (int pos = 0; pos < N; pos++) { // 让每个位置依次成为第一个不同字节
    unsigned char saved = right[pos]; // 暂存右数组当前位置的原值

    right[pos] = (unsigned char)(left[pos] - 1);
    assert(memcmp(left, right, (size_t)pos) == 0); // 比较长度不包含差异位置时应相等
    assert(memcmp(left, right, (size_t)pos + 1) > 0); // 包含差异位置后左数组应更大

    right[pos] = (unsigned char)(left[pos] + 1); // 让右数组当前位置的字节更大
    assert(memcmp(left, right, (size_t)pos + 1) < 0);

    right[pos] = saved; // 恢复右数组，供下一轮测试使用
  } 

  left[0] = 0x80; // 设置最高位为 1 的字节
  right[0] = 0x7f; // 设置最高位为 0 且数值较小的字节
  assert(memcmp(left, right, 1) > 0); // memcmp 应按 unsigned char 比较，左侧更大
  assert(memcmp(right, left, 1) < 0);

  left[0] = 0x20; // 在开头设置一个较小的左侧字节
  right[0] = 0x21; // 在开头设置一个较大的右侧字节
  left[N - 1] = 0xff; // 在末尾设置一个较大的左侧字节
  right[N - 1] = 0x00; // 在末尾设置一个较小的右侧字节
  assert(memcmp(left, right, N) < 0); // 开头先出现的差异决定整体比较结果
}

static void test_strlen(void) {
  char text[N + 1]; // 测试字符串缓冲区，额外留一个字节放末尾的零
  char original[N + 1]; // 保存调用前的内容，用于检查 strlen 没有修改输入

  for (int len = 0; len <= N; len++) {
    for (int i = 0; i <= N; i++) {
      text[i] = (char)('A' + i % 26);
    }

    text[len] = '\0';
    for (int i = 0; i <= N; i++) {
      original[i] = text[i];
    }

    assert(strlen(text) == (size_t)len); // 字符串长度应等于零所在位置
    for (int i = 0; i <= N; i++) {
      assert(text[i] == original[i]); // strlen 不应修改输入内容
    }
  }
} 

static void test_sprintf_d(void) { // 测试 sprintf 的 %d 十进制格式
  const int values[] = {
    0, INT_MAX / 17, INT_MAX, INT_MIN, INT_MIN + 1, // 零、正数和有符号边界
    UINT_MAX / 17, -1, 1 // 较大的正数、负数和正数
  };

  const char *expected[] = {
    "0", "126322567", "2147483647", "-2147483648",
    "-2147483647", "252645135", "-1", "1"
  };

  const size_t expected_len[] = {1, 9, 10, 11, 11, 9, 2, 1};
  char actual[sizeof(int) * CHAR_BIT + 2]; // 给最大 int 十进制结果留出足够空间

  size_t count = sizeof(values) / sizeof(values[0]);
  for (size_t i = 0; i < count; i++) {
    for (size_t j = 0; j < sizeof(actual); j++) { // 先填充整个输出缓冲区
      actual[j] = '#'; // 用非零字符作为哨兵，便于发现缺失的字符串结束符
    }

    int written = sprintf(actual, "%d", values[i]);
    assert(written == (int)expected_len[i]); // 返回值应是输出字符数，不包含结尾零
    assert(actual[expected_len[i]] == '\0');
    assert(strcmp(actual, expected[i]) == 0);
  }
}

int main(const char *args) {
  (void)args;
  test_memset();
  test_memcpy();
  test_memcmp();
  test_strlen();
  test_sprintf_d();
  return 0;
}
