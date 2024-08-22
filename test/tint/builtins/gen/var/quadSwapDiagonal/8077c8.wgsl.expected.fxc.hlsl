SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float2 quadSwapDiagonal_8077c8() {
  float2 arg_0 = (1.0f).xx;
  float2 res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_8077c8()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_8077c8()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000020A9F71CFD0(5,16-44): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'

