SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupAdd_1eb429() {
  int2 arg_0 = (int(1)).xx;
  int2 res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1eb429()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1eb429()));
}

FXC validation failure:
<scrubbed_path>(5,14-33): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
