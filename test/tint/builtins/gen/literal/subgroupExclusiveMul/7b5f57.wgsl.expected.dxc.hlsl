//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupExclusiveMul_7b5f57() {
  float4 res = WavePrefixProduct((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_7b5f57()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupExclusiveMul_7b5f57() {
  float4 res = WavePrefixProduct((1.0f).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_7b5f57()));
}

