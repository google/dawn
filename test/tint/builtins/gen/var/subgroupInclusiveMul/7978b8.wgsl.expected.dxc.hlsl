//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupInclusiveMul_7978b8() {
  float3 arg_0 = (1.0f).xxx;
  float3 v = arg_0;
  float3 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_7978b8()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupInclusiveMul_7978b8() {
  float3 arg_0 = (1.0f).xxx;
  float3 v = arg_0;
  float3 res = (WavePrefixProduct(v) * v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_7978b8()));
}

