
RWByteAddressBuffer prevent_dce : register(u0);
int subgroupMax_6c913e() {
  int res = WaveActiveMax(int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_6c913e()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_6c913e()));
}

