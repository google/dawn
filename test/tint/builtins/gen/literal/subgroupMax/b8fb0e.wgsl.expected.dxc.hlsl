//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupMax_b8fb0e() {
  uint2 res = WaveActiveMax((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupMax_b8fb0e());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupMax_b8fb0e() {
  uint2 res = WaveActiveMax((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupMax_b8fb0e());
}

