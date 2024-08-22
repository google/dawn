SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupShuffleDown_d46304() {
  uint4 res = WaveReadLaneAt((1u).xxxx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupShuffleDown_d46304());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupShuffleDown_d46304());
}

FXC validation failure:
C:\src\dawn\Shader@0x000001762192FDF0(4,42-59): error X3004: undeclared identifier 'WaveGetLaneIndex'

