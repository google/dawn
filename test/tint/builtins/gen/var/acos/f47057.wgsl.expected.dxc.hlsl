RWByteAddressBuffer prevent_dce : register(u0, space2);

void acos_f47057() {
  vector<float16_t, 3> arg_0 = (float16_t(0.96875h)).xxx;
  vector<float16_t, 3> res = acos(arg_0);
  prevent_dce.Store<vector<float16_t, 3> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  acos_f47057();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  acos_f47057();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_f47057();
  return;
}
