static int I;

[numthreads(1, 1, 1)]
void main() {
  I = 123;
  I = ((100 + 20) + 3);
  return;
}

