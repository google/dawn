SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupShuffle_84f261() {
  uint4 res = WaveReadLaneAt((1u).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_84f261()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_84f261()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001CDFE2A5050(4,15-43): error X3004: undeclared identifier 'WaveReadLaneAt'

