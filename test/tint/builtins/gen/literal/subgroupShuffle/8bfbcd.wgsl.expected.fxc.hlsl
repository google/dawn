SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupShuffle_8bfbcd() {
  int res = WaveReadLaneAt(1, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_8bfbcd()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_8bfbcd()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000021A8A8C0540(4,13-32): error X3004: undeclared identifier 'WaveReadLaneAt'

