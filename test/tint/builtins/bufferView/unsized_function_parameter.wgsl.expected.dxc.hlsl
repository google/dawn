
RWByteAddressBuffer v : register(u0);
void foo(uint tint_array_length) {
  v.Store((4u + (select((tint_array_length < 8u), 0u, 0u) * 1u)), 1077936128u);
}

[numthreads(1, 1, 1)]
void main() {
  uint v_1 = 0u;
  v.GetDimensions(v_1);
  foo(v_1);
}

