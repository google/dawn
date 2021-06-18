groupshared int4 arg_1;

void frexp_b87f4e() {
  float4 tint_tmp;
  float4 tint_tmp_1 = frexp(float4(0.0f, 0.0f, 0.0f, 0.0f), tint_tmp);
  arg_1 = int4(tint_tmp);
  float4 res = tint_tmp_1;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_b87f4e();
  return;
}
