SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupBroadcast_f637f9() {
  int4 res = WaveReadLaneAt((1).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_f637f9()));
}

FXC validation failure:
c:\src\dawn\Shader@0x000001C3208DCB30(4,14-41): error X3004: undeclared identifier 'WaveReadLaneAt'

