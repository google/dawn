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
