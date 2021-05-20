static int i = 123;

[numthreads(1, 1, 1)]
void main() {
  const int use = (i + 1);
  return;
}

