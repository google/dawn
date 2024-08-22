SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint2 quadSwapY_a27e1c() {
  uint2 arg_0 = (1u).xx;
  uint2 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, quadSwapY_a27e1c());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, quadSwapY_a27e1c());
}

FXC validation failure:
C:\src\dawn\Shader@0x00000181C8B14680(5,15-36): error X3004: undeclared identifier 'QuadReadAcrossY'

