SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupBroadcast_1d79c7() {
  int res = WaveReadLaneAt(1, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_1d79c7()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000017D40F3C6A0(4,13-33): error X3004: undeclared identifier 'WaveReadLaneAt'

