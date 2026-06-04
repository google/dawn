
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void main() {
  uint offset = 16u;
  uint v_2 = (offset & 4294967280u);
  uint v_3 = 0u;
  v.GetDimensions(v_3);
  uint v_4 = (16u + v_2);
  v_1.Store4(0u, v.Load4((0u + (select((v_3 < select((v_4 < 16u), 4294967295u, v_4)), 0u, v_2) * 1u))));
}

