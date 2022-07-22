[numthreads(1, 1, 1)]
void f() {
  const int a = 4;
  const int3 b = int3(1, 2, 3);
  const int3 r = (a % (b == int3(0, 0, 0) ? int3(1, 1, 1) : b));
  return;
}
