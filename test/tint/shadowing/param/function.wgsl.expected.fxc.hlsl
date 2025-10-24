
void a(int a_1) {
  int a_2 = a_1;
  int b = a_2;
}

[numthreads(1, 1, 1)]
void main() {
  a(int(1));
}

