SKIP: INVALID

RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1 = 0u;
  s.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = (tint_symbol_1 / 4u);
  uint q = 0u;
  matrix<float16_t, 3, 2> tint_symbol_3[2] = {matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(0.0h), float16_t(1.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h))), matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(0.0h), float16_t(1.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h)))};
  s.Store((4u * min(0u, (tint_symbol_2 - 1u))), asuint(uint(tint_symbol_3[min(q, 1u)][0][0])));
  return;
}
