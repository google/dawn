int func(int value, inout int pointer) {
  return (value + pointer);
}

[numthreads(1, 1, 1)]
void main() {
  int i = 123;
  const int r = func(i, i);
  return;
}
