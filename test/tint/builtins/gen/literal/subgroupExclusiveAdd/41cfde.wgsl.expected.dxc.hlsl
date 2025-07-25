//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupExclusiveAdd_41cfde() {
  float3 res = WavePrefixSum((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_41cfde()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupExclusiveAdd_41cfde() {
  float3 res = WavePrefixSum((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_41cfde()));
}

