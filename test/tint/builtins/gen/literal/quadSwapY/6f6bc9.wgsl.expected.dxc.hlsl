//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float quadSwapY_6f6bc9() {
  float res = QuadReadAcrossY(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapY_6f6bc9()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float quadSwapY_6f6bc9() {
  float res = QuadReadAcrossY(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapY_6f6bc9()));
}

