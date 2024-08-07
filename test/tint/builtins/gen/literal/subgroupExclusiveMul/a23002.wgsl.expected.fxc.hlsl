SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupExclusiveMul_a23002() {
  int res = WavePrefixProduct(1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_a23002()));
  return;
}
