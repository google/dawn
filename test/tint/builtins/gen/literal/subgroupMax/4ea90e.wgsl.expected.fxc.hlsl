SKIP: Wave ops not support before SM6.0

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupMax_4ea90e() {
  int3 res = WaveActiveMax((1).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_4ea90e()));
  return;
}
