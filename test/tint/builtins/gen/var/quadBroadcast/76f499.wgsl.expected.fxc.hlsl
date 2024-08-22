SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 quadBroadcast_76f499() {
  int4 arg_0 = (1).xxxx;
  int4 res = QuadReadLaneAt(arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_76f499()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_76f499()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000245DB5C46E0(5,14-37): error X3004: undeclared identifier 'QuadReadLaneAt'

