[numthreads(1, 1, 1)]
void f() {
  int a = 4;
  int3 b = int3(0, 2, 0);
  const int3 r = (a / (b == int3(0, 0, 0) ? int3(1, 1, 1) : b));
  return;
}
