struct S {
  int a;
};

ConstantBuffer<S> v : register(b0, space0);

[numthreads(1, 1, 1)]
void main() {
  const int use = (v.a + 1);
  return;
}

