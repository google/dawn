SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupBroadcast_3e6879() {
  int2 res = WaveReadLaneAt((1).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_3e6879()));
}

FXC validation failure:
c:\src\dawn\Shader@0x000002191E2F3970(4,14-39): error X3004: undeclared identifier 'WaveReadLaneAt'

