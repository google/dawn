
RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupMin_8ffadc() {
  float16_t res = WaveActiveMin(float16_t(1.0h));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupMin_8ffadc());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupMin_8ffadc());
}

