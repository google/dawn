SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupBroadcast_279027() {
  uint4 res = WaveReadLaneAt((1u).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupBroadcast_279027());
}

FXC validation failure:
c:\src\dawn\Shader@0x000001B3A4DF7C30(4,15-43): error X3004: undeclared identifier 'WaveReadLaneAt'

