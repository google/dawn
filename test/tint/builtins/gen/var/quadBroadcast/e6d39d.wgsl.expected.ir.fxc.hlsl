SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float quadBroadcast_e6d39d() {
  float arg_0 = 1.0f;
  float res = QuadReadLaneAt(arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_e6d39d()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_e6d39d()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000019F0DEF6A10(5,15-38): error X3004: undeclared identifier 'QuadReadLaneAt'

