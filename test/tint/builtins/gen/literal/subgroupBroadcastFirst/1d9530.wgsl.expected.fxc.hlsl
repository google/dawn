SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupBroadcastFirst_1d9530() {
  uint2 res = WaveReadLaneFirst((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_1d9530()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_1d9530()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000017689CEE1D0(4,15-40): error X3004: undeclared identifier 'WaveReadLaneFirst'

