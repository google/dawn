
[numthreads(1, 1, 1)]
void f0() {
  int a = int(2147483647);
  int b = int(1);
  int c = (a + int(1));
}

void f1() {
  int a = int(1);
  int b = (int(-2147483648) - a);
}

