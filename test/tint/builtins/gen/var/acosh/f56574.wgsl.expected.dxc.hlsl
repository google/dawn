vector<float16_t, 3> tint_acosh(vector<float16_t, 3> x) {
  return log((x + sqrt(((x * x) - float16_t(1.0h)))));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void acosh_f56574() {
  vector<float16_t, 3> arg_0 = (float16_t(1.54296875h)).xxx;
  vector<float16_t, 3> res = tint_acosh(arg_0);
  prevent_dce.Store<vector<float16_t, 3> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  acosh_f56574();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  acosh_f56574();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acosh_f56574();
  return;
}
