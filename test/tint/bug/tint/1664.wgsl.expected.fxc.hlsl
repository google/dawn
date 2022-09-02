[numthreads(1, 1, 1)]
void f0() {
  const int b = 1;
  const int c = -2147483648;
  return;
}

void f1() {
  const int b = 2147483647;
}
