int f(int a, int b, int c) {
  return ((a * b) + c);
}

void phony_sink(int p0, int p1, int p2) {
}

[numthreads(1, 1, 1)]
void main() {
  phony_sink(f(1, 2, 3), f(4, 5, 6), f(7, f(8, 9, 10), 11));
  return;
}
