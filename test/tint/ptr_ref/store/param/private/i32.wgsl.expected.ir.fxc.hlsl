
static int P = 0;
void func(inout int pointer) {
  pointer = 42;
}

[numthreads(1, 1, 1)]
void main() {
  func(P);
}

