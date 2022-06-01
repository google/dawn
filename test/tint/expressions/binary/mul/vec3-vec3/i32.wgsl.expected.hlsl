[numthreads(1, 1, 1)]
void f() {
  const int3 a = int3(1, 2, 3);
  const int3 b = int3(4, 5, 6);
  const int3 r = (int3(1, 2, 3) * int3(4, 5, 6));
  return;
}
