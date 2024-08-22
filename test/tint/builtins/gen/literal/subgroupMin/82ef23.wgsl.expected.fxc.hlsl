SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupMin_82ef23() {
  uint4 res = WaveActiveMin((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_82ef23()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_82ef23()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000021D67916440(4,15-38): error X3004: undeclared identifier 'WaveActiveMin'

