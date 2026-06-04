
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void main() {
  uint offset = 16u;
  uint v_2 = (offset & 4294967280u);
  uint v_3 = 0u;
  v.GetDimensions(v_3);
  uint v_4 = v_3;
  uint v_5 = (16u + v_2);
  bool v_6 = (v_4 < (((v_5 < 16u)) ? (4294967295u) : (v_5)));
  uint v_7 = ((v_6) ? (0u) : (v_2));
  uint v_8 = ((((v_6) ? (16u) : (16u)) / 16u) - 1u);
  v_1.Store4(0u, v.Load4(((0u + (v_7 * 1u)) + (min(uint(int(0)), v_8) * 16u))));
}

