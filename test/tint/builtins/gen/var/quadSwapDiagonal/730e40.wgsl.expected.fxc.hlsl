SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint quadSwapDiagonal_730e40() {
  uint arg_0 = 1u;
  uint res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_730e40()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_730e40()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002C675276170(5,14-42): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'

