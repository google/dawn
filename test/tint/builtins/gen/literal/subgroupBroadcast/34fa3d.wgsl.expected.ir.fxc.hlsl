SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupBroadcast_34fa3d() {
  uint3 res = WaveReadLaneAt((1u).xxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupBroadcast_34fa3d());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupBroadcast_34fa3d());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000020617FE9290(4,15-42): error X3004: undeclared identifier 'WaveReadLaneAt'

