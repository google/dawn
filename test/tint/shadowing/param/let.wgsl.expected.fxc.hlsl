
void f(int a) {
  int a_1 = a;
  int b = a_1;
}

[numthreads(1, 1, 1)]
void main() {
  f(int(1));
}

