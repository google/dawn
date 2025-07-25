//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupMul_2941a2() {
  float16_t res = WaveActiveProduct(float16_t(1.0h));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupMul_2941a2());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupMul_2941a2() {
  float16_t res = WaveActiveProduct(float16_t(1.0h));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupMul_2941a2());
}

