
void a(inout int x) {
}

void b(inout int x) {
  a(x);
}

[numthreads(1, 1, 1)]
void main() {
  int c = int(1);
  b(c);
}

