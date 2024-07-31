
int f(int a, int b, int c) {
  return ((a * b) + c);
}

[numthreads(1, 1, 1)]
void main() {
  int v = f(1, 2, 3);
  int v_1 = f(4, 5, 6);
}

