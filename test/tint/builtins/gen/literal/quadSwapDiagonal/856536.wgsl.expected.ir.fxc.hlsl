SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint3 quadSwapDiagonal_856536() {
  uint3 res = QuadReadAcrossDiagonal((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, quadSwapDiagonal_856536());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, quadSwapDiagonal_856536());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000018EBF6146B0(4,15-46): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'

