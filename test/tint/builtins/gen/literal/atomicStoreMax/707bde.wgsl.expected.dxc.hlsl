//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStoreMax_707bde() {
  uint64_t v = uint64_t(1u);
  uint64_t v_1 = uint64_t(1u);
  sb_rw.InterlockedMax64(0u, ((v_1 << uint64_t(32u)) | v));
}

void fragment_main() {
  atomicStoreMax_707bde();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStoreMax_707bde() {
  uint64_t v = uint64_t(1u);
  uint64_t v_1 = uint64_t(1u);
  sb_rw.InterlockedMax64(0u, ((v_1 << uint64_t(32u)) | v));
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStoreMax_707bde();
}

