[numthreads(1, 1, 1)]
void f() {
  int a = 1;
  int b = 0;
  const int r = (a % (b == 0 ? 1 : b));
  return;
}
