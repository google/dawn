SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const int c = (1 / 0);
  return;
}
O:\src\chrome\src\third_party\dawn\third_party\tint\test\Shader@0x000002477B82F5C0(3,18-22): error X4010: Unsigned integer divide by zero

