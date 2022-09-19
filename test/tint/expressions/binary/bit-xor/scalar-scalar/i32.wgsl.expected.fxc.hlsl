[numthreads(1, 1, 1)]
void f() {
  const int a = 1;
  const int b = 2;
  const int r = (a ^ b);
  return;
}
