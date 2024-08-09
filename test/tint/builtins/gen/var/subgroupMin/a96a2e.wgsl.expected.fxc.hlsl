SKIP: Wave ops not support before SM6.0

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupMin_a96a2e() {
  int arg_0 = 1;
  int res = WaveActiveMin(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_a96a2e()));
  return;
}
