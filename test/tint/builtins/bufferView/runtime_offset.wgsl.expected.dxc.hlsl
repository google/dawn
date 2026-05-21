
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void main() {
  uint offset = 16u;
  uint v_2 = 0u;
  v.GetDimensions(v_2);
  v_1.Store4(0u, v.Load4((0u + (select((v_2 < select(((16u + (offset & 4294967280u)) < 16u), 4294967295u, (16u + (offset & 4294967280u)))), 0u, (offset & 4294967280u)) * 1u))));
}

