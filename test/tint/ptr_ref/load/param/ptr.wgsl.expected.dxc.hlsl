
int func(int value, inout int pointer) {
  return (value + pointer);
}

[numthreads(1, 1, 1)]
void main() {
  int i = int(123);
  int r = func(i, i);
}

