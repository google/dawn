groupshared uint3 arg_1;

void frexp_6be229() {
  float3 tint_tmp;
  float3 tint_tmp_1 = frexp(float3(0.0f, 0.0f, 0.0f), tint_tmp);
  arg_1 = uint3(tint_tmp);
  float3 res = tint_tmp_1;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_6be229();
  return;
}
