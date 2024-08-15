SKIP: Wave ops not supported before SM 6.0


RWByteAddressBuffer prevent_dce : register(u0);
int quadSwapX_1e1086() {
  int res = QuadReadAcrossX(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_1e1086()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_1e1086()));
}

