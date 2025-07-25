//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupExclusiveAdd_4a1568() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupExclusiveAdd_4a1568());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupExclusiveAdd_4a1568() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupExclusiveAdd_4a1568());
}

