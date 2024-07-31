SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupBroadcast_c36fe1() {
  uint res = WaveReadLaneAt(1u, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupBroadcast_c36fe1());
}

FXC validation failure:
c:\src\dawn\Shader@0x000001D02F3403E0(4,14-35): error X3004: undeclared identifier 'WaveReadLaneAt'

