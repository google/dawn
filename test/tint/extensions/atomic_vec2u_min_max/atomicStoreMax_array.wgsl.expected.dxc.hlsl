
RWByteAddressBuffer sb_rw0 : register(u0);
RWByteAddressBuffer sb_rw1 : register(u1);
[numthreads(1, 1, 1)]
void compute_main() {
  uint2 in_vec = uint2(1u, 2u);
  uint2 v = in_vec;
  uint64_t v_1 = uint64_t(v.x);
  uint64_t v_2 = uint64_t(v.y);
  sb_rw0.InterlockedMax64(8u, ((v_2 << uint64_t(32u)) | v_1));
  uint v_3 = 0u;
  sb_rw1.GetDimensions(v_3);
  uint v_4 = (min(1u, ((v_3 / 8u) - 1u)) * 8u);
  uint2 v_5 = in_vec;
  uint64_t v_6 = uint64_t(v_5.x);
  uint64_t v_7 = uint64_t(v_5.y);
  sb_rw1.InterlockedMax64((0u + v_4), ((v_7 << uint64_t(32u)) | v_6));
}

