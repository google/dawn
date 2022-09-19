[numthreads(1, 1, 1)]
void f() {
  const int3 a = int3(1, 2, 3);
  const int b = 4;
  const int3 r = (a - b);
  return;
}
