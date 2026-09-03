
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void main() {
  uint v_1 = 0u;
  v.GetDimensions(v_1);
  bool v_2 = (v_1 < 64u);
  v.Store(((16u + (select(v_2, 0u, 0u) * 1u)) + (min(4u, (((select(v_2, 20u, 64u) - 16u) / 4u) - 1u)) * 4u)), 4u);
}

