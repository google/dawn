SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupAdd_1eb429() {
  int2 res = WaveActiveSum((1).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1eb429()));
  return;
}
