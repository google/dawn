//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int quadSwapY_94ab6d() {
  int res = QuadReadAcrossY(int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapY_94ab6d()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int quadSwapY_94ab6d() {
  int res = QuadReadAcrossY(int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapY_94ab6d()));
}

