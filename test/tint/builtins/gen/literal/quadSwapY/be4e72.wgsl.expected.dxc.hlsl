//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 quadSwapY_be4e72() {
  int3 res = QuadReadAcrossY((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_be4e72()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 quadSwapY_be4e72() {
  int3 res = QuadReadAcrossY((int(1)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_be4e72()));
}

