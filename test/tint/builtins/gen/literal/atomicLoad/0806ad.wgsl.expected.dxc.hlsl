RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicLoad(uint offset) {
  int value = 0;
  sb_rw.InterlockedOr(offset, 0, value);
  return value;
}


RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicLoad_0806ad() {
  int res = sb_rwatomicLoad(0u);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  atomicLoad_0806ad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicLoad_0806ad();
  return;
}
