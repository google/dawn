[numthreads(1, 1, 1)]
void f() {
  const int a = 1;
  const uint b = 2u;
  const int r = (a << (b & 31u));
  return;
}
