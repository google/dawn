SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int3 quadSwapDiagonal_a82e1d() {
  int3 res = QuadReadAcrossDiagonal((1).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_a82e1d()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_a82e1d()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000022B22E98600(4,14-44): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'

