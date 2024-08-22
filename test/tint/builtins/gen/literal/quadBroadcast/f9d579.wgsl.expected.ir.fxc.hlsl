SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int quadBroadcast_f9d579() {
  int res = QuadReadLaneAt(1, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_f9d579()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_f9d579()));
}

FXC validation failure:
C:\src\dawn\Shader@0x00000284B767F6E0(4,13-32): error X3004: undeclared identifier 'QuadReadLaneAt'

