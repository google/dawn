SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupBroadcast_4a4334() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_4a4334()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000015D73FA0A90(5,15-39): error X3004: undeclared identifier 'WaveReadLaneAt'

