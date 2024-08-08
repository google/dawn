SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBroadcast_279027() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_279027()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000024CFF3BFD20(5,15-39): error X3004: undeclared identifier 'WaveReadLaneAt'

