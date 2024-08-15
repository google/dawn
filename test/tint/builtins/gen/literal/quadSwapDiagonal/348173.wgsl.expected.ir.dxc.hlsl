
RWByteAddressBuffer prevent_dce : register(u0);
uint2 quadSwapDiagonal_348173() {
  uint2 res = QuadReadAcrossDiagonal((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, quadSwapDiagonal_348173());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, quadSwapDiagonal_348173());
}

