struct S {
  int i;
};

static S V;

[numthreads(1, 1, 1)]
void main() {
  V.i = 5;
  return;
}

