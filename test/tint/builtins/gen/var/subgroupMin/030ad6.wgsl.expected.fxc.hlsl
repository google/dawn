SKIP: Wave ops not support before SM6.0

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupMin_030ad6() {
  int3 arg_0 = (1).xxx;
  int3 res = WaveActiveMin(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMin_030ad6()));
  return;
}
