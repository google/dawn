
RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupMul_dd1333() {
  uint4 res = WaveActiveProduct((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupMul_dd1333());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupMul_dd1333());
}

