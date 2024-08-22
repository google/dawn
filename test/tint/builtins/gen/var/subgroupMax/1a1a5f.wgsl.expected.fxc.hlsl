SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupMax_1a1a5f() {
  float arg_0 = 1.0f;
  float res = WaveActiveMax(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_1a1a5f()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_1a1a5f()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000023128255350(5,15-34): error X3004: undeclared identifier 'WaveActiveMax'

