
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
  uint v_4 = v_3;
  uint v_5 = max(v_2, 16u);
  uint v_6 = (0u + v_5);
  bool v_7 = (v_4 < (((v_6 < 0u)) ? (4294967295u) : (v_6)));
  uint v_8 = ((v_7) ? (0u) : (0u));
  v_1.Store4(0u, v.Load4(((0u + (v_8 * 1u)) + (min(0u, ((((v_7) ? (16u) : (v_5)) / 16u) - 1u)) * 16u))));
}

