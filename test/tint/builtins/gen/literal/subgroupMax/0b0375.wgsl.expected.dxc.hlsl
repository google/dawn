//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupMax_0b0375() {
  float4 res = WaveActiveMax((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_0b0375()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupMax_0b0375() {
  float4 res = WaveActiveMax((1.0f).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_0b0375()));
}

