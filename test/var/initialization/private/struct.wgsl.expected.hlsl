struct S {
  int a;
  float b;
};

static S v = {0, 0.0f};

[numthreads(1, 1, 1)]
void main() {
  v;
  return;
}
