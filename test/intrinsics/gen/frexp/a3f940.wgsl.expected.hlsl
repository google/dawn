groupshared int2 arg_1;

void frexp_a3f940() {
  float2 tint_tmp;
  float2 tint_tmp_1 = frexp(float2(0.0f, 0.0f), tint_tmp);
  arg_1 = int2(tint_tmp);
  float2 res = tint_tmp_1;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_a3f940();
  return;
}
