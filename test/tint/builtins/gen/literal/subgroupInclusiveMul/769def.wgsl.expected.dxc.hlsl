//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupInclusiveMul_769def() {
  int3 res = asint((asuint(WavePrefixProduct((int(1)).xxx)) * (1u).xxx));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_769def()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupInclusiveMul_769def() {
  int3 res = asint((asuint(WavePrefixProduct((int(1)).xxx)) * (1u).xxx));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_769def()));
}

