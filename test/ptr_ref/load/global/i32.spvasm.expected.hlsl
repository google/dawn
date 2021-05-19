static int I = 0;

[numthreads(1, 1, 1)]
void main() {
  const int x_9 = I;
  const int x_11 = (x_9 + 1);
  return;
}

