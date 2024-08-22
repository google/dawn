SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupShuffleDown_d46304() {
  uint4 res = WaveReadLaneAt((1u).xxxx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_d46304()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_d46304()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002E2E0F262B0(4,42-59): error X3004: undeclared identifier 'WaveGetLaneIndex'

