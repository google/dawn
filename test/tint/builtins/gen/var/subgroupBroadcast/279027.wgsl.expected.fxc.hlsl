SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBroadcast_279027() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_279027()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_279027()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001A457BC60F0(5,15-39): error X3004: undeclared identifier 'WaveReadLaneAt'

