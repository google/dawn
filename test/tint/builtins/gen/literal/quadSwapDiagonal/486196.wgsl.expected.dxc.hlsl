RWByteAddressBuffer prevent_dce : register(u0);

float quadSwapDiagonal_486196() {
  float res = QuadReadAcrossDiagonal(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_486196()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_486196()));
  return;
}
