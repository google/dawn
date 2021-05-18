struct S {
  int i;
};

[numthreads(1, 1, 1)]
void main() {
  S V = {0};
  int i = V.i;
  return;
}

