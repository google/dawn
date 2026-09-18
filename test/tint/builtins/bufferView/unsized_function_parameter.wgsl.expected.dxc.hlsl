
RWByteAddressBuffer v : register(u0);
void foo() {
  uint v_1 = 0u;
  v.GetDimensions(v_1);
  v.Store((4u + (select((v_1 < 8u), 0u, 0u) * 1u)), 1077936128u);
}

[numthreads(1, 1, 1)]
void main() {
  foo();
}

