
RWByteAddressBuffer v : register(u0);
void foo() {
  uint v_1 = 0u;
  v.GetDimensions(v_1);
  bool v_2 = (v_1 < 32u);
  v.Store(((4u + (select(v_2, 0u, 0u) * 1u)) + (min(0u, ((select(v_2, 8u, 32u) / 8u) - 1u)) * 8u)), 1077936128u);
}

[numthreads(1, 1, 1)]
void main() {
  foo();
}

