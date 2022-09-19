[numthreads(1, 1, 1)]
void f0() {
  const int a = 2147483647;
  const int b = 1;
  const int c = (a + 1);
  return;
}

void f1() {
  const int a = 1;
  const int b = (-2147483648 - a);
}
