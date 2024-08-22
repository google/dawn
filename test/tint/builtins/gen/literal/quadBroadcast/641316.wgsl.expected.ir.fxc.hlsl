SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint2 quadBroadcast_641316() {
  uint2 res = QuadReadLaneAt((1u).xx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, quadBroadcast_641316());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, quadBroadcast_641316());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000022027F1FB80(4,15-40): error X3004: undeclared identifier 'QuadReadLaneAt'

