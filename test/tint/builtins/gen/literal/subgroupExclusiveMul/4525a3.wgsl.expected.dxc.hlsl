RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupExclusiveMul_4525a3() {
  int2 res = WavePrefixProduct((1).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_4525a3()));
  return;
}
