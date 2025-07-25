//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 quadSwapY_1f1a06() {
  float2 res = QuadReadAcrossY((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_1f1a06()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 quadSwapY_1f1a06() {
  float2 res = QuadReadAcrossY((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_1f1a06()));
}

