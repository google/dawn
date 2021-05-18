struct S {
  int i;
};

S V;

[numthreads(1, 1, 1)]
void main() {
  const int i = V.i;
  return;
}

