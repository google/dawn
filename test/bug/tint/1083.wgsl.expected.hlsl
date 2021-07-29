SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const int c = (1 / 0);
  return;
}
C:\src\tint\test\Shader@0x000001BC7A7DD8D0(3,18-22): error X4010: Unsigned integer divide by zero

