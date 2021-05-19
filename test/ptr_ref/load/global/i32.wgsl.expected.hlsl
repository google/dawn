static int I;

[numthreads(1, 1, 1)]
void main() {
  const int i = I;
  const int use = (i + 1);
  return;
}

