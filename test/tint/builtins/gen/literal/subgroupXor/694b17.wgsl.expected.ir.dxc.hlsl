
RWByteAddressBuffer prevent_dce : register(u0);
int subgroupXor_694b17() {
  int res = asint(WaveActiveBitXor(asuint(int(1))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_694b17()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_694b17()));
}

