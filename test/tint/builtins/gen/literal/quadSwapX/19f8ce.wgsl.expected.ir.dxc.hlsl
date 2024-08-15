
RWByteAddressBuffer prevent_dce : register(u0);
uint2 quadSwapX_19f8ce() {
  uint2 res = QuadReadAcrossX((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, quadSwapX_19f8ce());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, quadSwapX_19f8ce());
}

