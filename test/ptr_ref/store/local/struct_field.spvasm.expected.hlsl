struct S {
  int i;
};

[numthreads(1, 1, 1)]
void main() {
  S V = {0};
  V.i = 5;
  return;
}

