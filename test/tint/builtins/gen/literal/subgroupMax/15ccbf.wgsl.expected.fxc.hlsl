SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupMax_15ccbf() {
  uint4 res = WaveActiveMax((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_15ccbf()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_15ccbf()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000022CA3C267D0(4,15-38): error X3004: undeclared identifier 'WaveActiveMax'

