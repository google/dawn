
void f(int a) {
  int b = a;
}

[numthreads(1, 1, 1)]
void main() {
  f(int(1));
}

