static int I = 0;

void main_1() {
  const int x_9 = I;
  const int x_11 = (x_9 + 1);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
