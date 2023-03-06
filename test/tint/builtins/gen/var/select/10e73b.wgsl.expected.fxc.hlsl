SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void select_10e73b() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t arg_1 = float16_t(1.0h);
  bool arg_2 = true;
  float16_t res = (arg_2 ? arg_1 : arg_0);
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_10e73b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_10e73b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_10e73b();
  return;
}
