SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupMin_337a21() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMin_337a21()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMin_337a21()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000023AE6016030(5,15-34): error X3004: undeclared identifier 'WaveActiveMin'

