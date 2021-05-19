void func(int value, inout int pointer) {
  pointer = value;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  int i = 0;
  i = 123;
  func(123, i);
  return;
}

