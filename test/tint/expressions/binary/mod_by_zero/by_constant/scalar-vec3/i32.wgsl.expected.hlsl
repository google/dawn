[numthreads(1, 1, 1)]
void f() {
  const int a = 4;
  const int3 b = int3(0, 2, 0);
  const int3 r = (a % int3(1, 2, 1));
  return;
}
