groupshared int3 arg_1;

void frexp_40fc9b() {
  float3 tint_tmp;
  float3 tint_tmp_1 = frexp(float3(0.0f, 0.0f, 0.0f), tint_tmp);
  arg_1 = int3(tint_tmp);
  float3 res = tint_tmp_1;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_40fc9b();
  return;
}
