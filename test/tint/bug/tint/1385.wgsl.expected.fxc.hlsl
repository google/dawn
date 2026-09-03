
ByteAddressBuffer data : register(t1);
int foo() {
  uint v = 0u;
  data.GetDimensions(v);
  return asint(data.Load((0u + (min(0u, ((v / 4u) - 1u)) * 4u))));
}

[numthreads(16, 16, 1)]
void main() {
  foo();
}

