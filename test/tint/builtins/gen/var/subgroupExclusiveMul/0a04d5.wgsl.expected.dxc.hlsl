//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupExclusiveMul_0a04d5() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_0a04d5()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupExclusiveMul_0a04d5() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_0a04d5()));
}

