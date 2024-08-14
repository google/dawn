RWByteAddressBuffer prevent_dce : register(u0);

int4 quadSwapY_14bb9a() {
  int4 res = QuadReadAcrossY((1).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_14bb9a()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_14bb9a()));
  return;
}
