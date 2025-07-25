//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 quadSwapY_0d05a8() {
  int2 res = QuadReadAcrossY((int(1)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_0d05a8()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 quadSwapY_0d05a8() {
  int2 res = QuadReadAcrossY((int(1)).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_0d05a8()));
}

