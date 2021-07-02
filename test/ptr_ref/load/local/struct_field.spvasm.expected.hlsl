struct S {
  int i;
};

void main_1() {
  int i = 0;
  S V = (S)0;
  const int x_14 = V.i;
  i = x_14;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
