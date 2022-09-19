[numthreads(1, 1, 1)]
void f() {
  const uint a = 1u;
  const uint b = asuint(a);
  return;
}
