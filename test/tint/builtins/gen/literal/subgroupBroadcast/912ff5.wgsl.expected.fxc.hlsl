SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupBroadcast_912ff5() {
  float3 res = WaveReadLaneAt((1.0f).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_912ff5()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001F7AC5AEE90(4,16-45): error X3004: undeclared identifier 'WaveReadLaneAt'

