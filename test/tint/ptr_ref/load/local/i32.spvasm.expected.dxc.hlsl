void main_1() {
  int i = 0;
  i = 123;
  const int x_10 = i;
  const int x_12 = (x_10 + 1);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
