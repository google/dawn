SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int quadSwapDiagonal_9ccb38() {
  int res = QuadReadAcrossDiagonal(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_9ccb38()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_9ccb38()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000014278CBAFA0(4,13-37): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'

