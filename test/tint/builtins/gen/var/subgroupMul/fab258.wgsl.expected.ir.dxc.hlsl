
RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupMul_fab258() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = WaveActiveProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_fab258()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_fab258()));
}

