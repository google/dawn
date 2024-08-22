SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupMin_0bc13a() {
  int2 arg_0 = (1).xx;
  int2 res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMin_0bc13a()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMin_0bc13a()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002615D7C9B60(5,14-33): error X3004: undeclared identifier 'WaveActiveMin'

