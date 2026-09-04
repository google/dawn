
RWByteAddressBuffer a : register(u0);
void main() {
  uint v = 0u;
  a.GetDimensions(v);
  uint v_1 = (min(0u, ((v / 8u) - 1u)) * 8u);
  uint64_t v_2 = uint64_t(0u);
  uint64_t v_3 = uint64_t(0u);
  a.InterlockedMin64((0u + v_1), ((v_3 << uint64_t(32u)) | v_2));
}

