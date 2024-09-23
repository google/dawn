
RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupMin_030ad6() {
  int3 res = WaveActiveMin((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMin_030ad6()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMin_030ad6()));
}

