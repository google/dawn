SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupBroadcast_e275c8() {
  int3 res = WaveReadLaneAt((1).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_e275c8()));
}

FXC validation failure:
c:\src\dawn\Shader@0x00000252C885F2D0(4,14-40): error X3004: undeclared identifier 'WaveReadLaneAt'

