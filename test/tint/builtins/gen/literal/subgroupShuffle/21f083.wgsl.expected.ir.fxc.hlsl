SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupShuffle_21f083() {
  uint2 res = WaveReadLaneAt((1u).xx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupShuffle_21f083());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupShuffle_21f083());
}

FXC validation failure:
C:\src\dawn\Shader@0x000001D71676EF00(4,15-40): error X3004: undeclared identifier 'WaveReadLaneAt'

