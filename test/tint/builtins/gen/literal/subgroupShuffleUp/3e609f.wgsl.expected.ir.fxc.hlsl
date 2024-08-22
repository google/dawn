SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupShuffleUp_3e609f() {
  int4 res = WaveReadLaneAt((1).xxxx, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_3e609f()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_3e609f()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000001EF62EF0C80(4,40-57): error X3004: undeclared identifier 'WaveGetLaneIndex'

