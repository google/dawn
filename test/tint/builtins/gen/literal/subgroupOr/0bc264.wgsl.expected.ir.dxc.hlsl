
RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupOr_0bc264() {
  uint res = WaveActiveBitOr(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupOr_0bc264());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupOr_0bc264());
}

