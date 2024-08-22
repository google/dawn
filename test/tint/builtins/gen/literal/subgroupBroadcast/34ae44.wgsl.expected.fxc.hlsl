SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupBroadcast_34ae44() {
  uint3 res = WaveReadLaneAt((1u).xxx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_34ae44()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_34ae44()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000018FA5C2FFD0(4,15-41): error X3004: undeclared identifier 'WaveReadLaneAt'

