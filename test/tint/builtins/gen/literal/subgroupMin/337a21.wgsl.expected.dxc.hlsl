//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupMin_337a21() {
  uint3 res = WaveActiveMin((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupMin_337a21());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupMin_337a21() {
  uint3 res = WaveActiveMin((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupMin_337a21());
}

