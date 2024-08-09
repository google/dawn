SKIP: Wave ops not support before SM6.0

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupMax_7e81ea() {
  float3 res = WaveActiveMax((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_7e81ea()));
  return;
}
