static volatile unsigned int spin_count = 0; // 保存循环次数，并要求编译器保留对它的访问

int main(const char *args) {
  (void)args;

  while (1) { 
    spin_count++; // 每轮修改计数器，形成真实的客户指令执行
  }

  return 0;
}