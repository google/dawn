
RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint arrayLength_cbd6b5() {
  uint v = 0u;
  sb_rw.GetDimensions(v);
  uint res = (v / 2u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, arrayLength_cbd6b5());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, arrayLength_cbd6b5());
}

