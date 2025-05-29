
RWByteAddressBuffer s : register(u0);
uint tint_f16_to_u32(float16_t value) {
  return (((value <= float16_t(65504.0h))) ? ((((value >= float16_t(0.0h))) ? (uint(value)) : (0u))) : (4294967295u));
}

[numthreads(1, 1, 1)]
void main() {
  uint q = 0u;
  uint v = 0u;
  s.GetDimensions(v);
  uint v_1 = ((v / 4u) - 1u);
  uint v_2 = (min(uint(int(0)), v_1) * 4u);
  matrix<float16_t, 3, 2> v_3[2] = {matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(0.0h), float16_t(1.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h))), matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(0.0h), float16_t(1.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h)))};
  s.Store((0u + v_2), tint_f16_to_u32(v_3[min(q, 1u)][0u].x));
}

