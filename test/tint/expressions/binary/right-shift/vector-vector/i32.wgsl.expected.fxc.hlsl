[numthreads(1, 1, 1)]
void f() {
  const int3 a = int3(1, 2, 3);
  const uint3 b = uint3(4u, 5u, 6u);
  const int3 r = (a >> (b & (31u).xxx));
  return;
}
