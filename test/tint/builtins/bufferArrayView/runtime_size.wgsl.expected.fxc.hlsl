
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / (((rhs == 0u)) ? (1u) : (rhs)));
}

void main() {
  uint size = 16u;
  uint v_2 = (tint_div_u32(size, 16u) * 16u);
  uint v_3 = 0u;
  v.GetDimensions(v_3);
  uint v_4 = max(v_2, 16u);
  bool v_5 = (v_3 < v_4);
  uint v_6 = ((v_5) ? (0u) : (0u));
  uint v_7 = ((((v_5) ? (16u) : (v_4)) / 16u) - 1u);
  v_1.Store4(0u, v.Load4(((0u + (v_6 * 1u)) + (min(uint(int(0)), v_7) * 16u))));
}

