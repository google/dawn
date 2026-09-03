
RWByteAddressBuffer arr : register(u0);
uint f2(uint tint_array_length) {
  return tint_array_length;
}

uint f1(uint tint_array_length) {
  return f2(tint_array_length);
}

uint f0(uint tint_array_length) {
  return f1(tint_array_length);
}

[numthreads(1, 1, 1)]
void main() {
  uint v = 0u;
  arr.GetDimensions(v);
  uint v_1 = (min(0u, ((v / 4u) - 1u)) * 4u);
  uint v_2 = 0u;
  arr.GetDimensions(v_2);
  arr.Store((0u + v_1), f0((v_2 / 4u)));
}

