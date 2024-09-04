
int f(int a, int b, int c) {
  return ((a * b) + c);
}

[numthreads(1, 1, 1)]
void main() {
  int v = f(1, 2, 3);
  int v_1 = f(4, 5, 6);
  int v_2 = (v + (v_1 * f(7, f(8, 9, 10), 11)));
}

