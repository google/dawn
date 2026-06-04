
RWByteAddressBuffer v : register(u0);
void foo(uint tint_array_length) {
  bool v_1 = (tint_array_length < 32u);
  uint v_2 = ((v_1) ? (0u) : (0u));
  uint v_3 = ((((v_1) ? (8u) : (32u)) / 8u) - 1u);
  v.Store(((4u + (v_2 * 1u)) + (min(uint(int(0)), v_3) * 8u)), asuint(3.0f));
}

[numthreads(1, 1, 1)]
void main() {
  uint v_4 = 0u;
  v.GetDimensions(v_4);
  foo(v_4);
}

