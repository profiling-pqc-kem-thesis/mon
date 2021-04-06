#include <stdio.h>
#include <unistd.h>

void func() {
  printf("Hello, World!\n");
  sleep(1);
}

int main(int argc, char **argv) {
  printf("Running from Child\n");
  for (int i = 0; i < 5; i++)
    func();
  return 0;
}
