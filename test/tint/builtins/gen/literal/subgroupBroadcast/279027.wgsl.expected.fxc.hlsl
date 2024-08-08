SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBroadcast_279027() {
  uint4 res = WaveReadLaneAt((1u).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_279027()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002212FF5C630(4,15-43): error X3004: undeclared identifier 'WaveReadLaneAt'

