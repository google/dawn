struct S {
  int i;
};

[numthreads(1, 1, 1)]
void main() {
  int i = 0;
  S V = {0};
  const int x_14 = V.i;
  i = x_14;
  return;
}

