RWByteAddressBuffer prevent_dce : register(u0, space2);

void tanh_6d105a() {
  vector<float16_t, 2> res = (float16_t(0.76123046875h)).xx;
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  tanh_6d105a();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  tanh_6d105a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_6d105a();
  return;
}
