struct S {
  int i;
};

static S V = (S)0;

void main_1() {
  int i = 0;
  const int x_15 = V.i;
  i = x_15;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
