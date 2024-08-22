SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 quadBroadcast_f5f923() {
  int2 res = QuadReadLaneAt((1).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_f5f923()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_f5f923()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000026AB8EF46A0(4,14-39): error X3004: undeclared identifier 'QuadReadLaneAt'

