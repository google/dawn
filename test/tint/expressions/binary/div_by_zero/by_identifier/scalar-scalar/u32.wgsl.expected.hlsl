[numthreads(1, 1, 1)]
void f() {
  uint a = 1u;
  uint b = 0u;
  const uint r = (a / (b == 0u ? 1u : b));
  return;
}
