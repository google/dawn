//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupInclusiveMul_89437b() {
  uint res = (WavePrefixProduct(1u) * 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupInclusiveMul_89437b());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupInclusiveMul_89437b() {
  uint res = (WavePrefixProduct(1u) * 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupInclusiveMul_89437b());
}

