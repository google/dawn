[numthreads(1, 1, 1)]
void f0() {
  int a = 2147483647;
  int b = 1;
  int c = (a + 1);
  return;
}

void f1() {
  int a = 1;
  int b = (-2147483648 - a);
}
