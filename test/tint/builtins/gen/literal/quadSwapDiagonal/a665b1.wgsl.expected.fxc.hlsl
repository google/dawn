SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 quadSwapDiagonal_a665b1() {
  int4 res = QuadReadAcrossDiagonal((1).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_a665b1()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_a665b1()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000026F1498A5A0(4,14-45): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'

