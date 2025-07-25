//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float quadSwapX_9bea80() {
  float arg_0 = 1.0f;
  float res = QuadReadAcrossX(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_9bea80()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float quadSwapX_9bea80() {
  float arg_0 = 1.0f;
  float res = QuadReadAcrossX(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_9bea80()));
}

