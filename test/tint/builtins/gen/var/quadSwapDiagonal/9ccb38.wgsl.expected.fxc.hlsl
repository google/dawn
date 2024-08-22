SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int quadSwapDiagonal_9ccb38() {
  int arg_0 = 1;
  int res = QuadReadAcrossDiagonal(arg_0);
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
C:\src\dawn\Shader@0x000001420DF36BB0(5,13-41): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'

