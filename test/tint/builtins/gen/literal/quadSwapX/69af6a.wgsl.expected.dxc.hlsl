//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quadSwapX_69af6a() {
  float4 res = QuadReadAcrossX((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapX_69af6a()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quadSwapX_69af6a() {
  float4 res = QuadReadAcrossX((1.0f).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapX_69af6a()));
}

