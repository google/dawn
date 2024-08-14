SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupMin_337a21() {
  uint3 res = WaveActiveMin((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupMin_337a21());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupMin_337a21());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000028A1EF6F0A0(4,15-37): error X3004: undeclared identifier 'WaveActiveMin'

