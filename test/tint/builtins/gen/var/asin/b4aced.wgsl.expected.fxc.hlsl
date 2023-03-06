SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void asin_b4aced() {
  vector<float16_t, 2> arg_0 = (float16_t(0.479248046875h)).xx;
  vector<float16_t, 2> res = asin(arg_0);
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  asin_b4aced();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  asin_b4aced();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_b4aced();
  return;
}
