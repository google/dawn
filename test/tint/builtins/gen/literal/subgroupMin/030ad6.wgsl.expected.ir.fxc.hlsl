SKIP: INVALID


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

FXC validation failure:
<scrubbed_path>(4,14-40): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
