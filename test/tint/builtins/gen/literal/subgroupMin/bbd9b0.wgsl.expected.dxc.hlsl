//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupMin_bbd9b0() {
  float4 res = WaveActiveMin((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_bbd9b0()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupMin_bbd9b0() {
  float4 res = WaveActiveMin((1.0f).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_bbd9b0()));
}

