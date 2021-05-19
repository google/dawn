struct S {
  int i;
};

static S V;

[numthreads(1, 1, 1)]
void main() {
  int i = 0;
  const int x_15 = V.i;
  i = x_15;
  return;
}

