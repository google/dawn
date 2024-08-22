SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 quadBroadcast_bed00b() {
  int4 res = QuadReadLaneAt((1).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_bed00b()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_bed00b()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000232144E89B0(4,14-41): error X3004: undeclared identifier 'QuadReadLaneAt'

