struct S {
  int a;
  float b;
};

[numthreads(1, 1, 1)]
void main() {
  S v = {0, 0.0f};
  v;
  return;
}
