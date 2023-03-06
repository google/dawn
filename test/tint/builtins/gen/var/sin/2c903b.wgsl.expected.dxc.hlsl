RWByteAddressBuffer prevent_dce : register(u0, space2);

void sin_2c903b() {
  vector<float16_t, 3> arg_0 = (float16_t(1.5703125h)).xxx;
  vector<float16_t, 3> res = sin(arg_0);
  prevent_dce.Store<vector<float16_t, 3> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  sin_2c903b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  sin_2c903b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sin_2c903b();
  return;
}
