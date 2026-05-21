
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void main() {
  uint offset = 16u;
  uint v_2 = 0u;
  v.GetDimensions(v_2);
  uint v_3 = v_2;
  bool v_4 = (v_3 < ((((16u + (offset & 4294967280u)) < 16u)) ? (4294967295u) : ((16u + (offset & 4294967280u)))));
  uint v_5 = ((v_4) ? (0u) : ((offset & 4294967280u)));
  uint v_6 = ((((v_4) ? (16u) : (16u)) / 16u) - 1u);
  v_1.Store4(0u, v.Load4(((0u + (v_5 * 1u)) + (min(uint(int(0)), v_6) * 16u))));
}

