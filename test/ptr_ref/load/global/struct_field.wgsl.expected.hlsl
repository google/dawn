struct S {
  int i;
};

static S V;

[numthreads(1, 1, 1)]
void main() {
  const int i = V.i;
  return;
}

