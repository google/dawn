SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float2 quadSwapY_1f1a06() {
  float2 arg_0 = (1.0f).xx;
  float2 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_1f1a06()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_1f1a06()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000019C87ADB9B0(5,16-37): error X3004: undeclared identifier 'QuadReadAcrossY'

