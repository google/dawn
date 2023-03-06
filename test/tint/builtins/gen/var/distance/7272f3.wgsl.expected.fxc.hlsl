SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void distance_7272f3() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  vector<float16_t, 4> arg_1 = (float16_t(1.0h)).xxxx;
  float16_t res = distance(arg_0, arg_1);
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  distance_7272f3();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  distance_7272f3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  distance_7272f3();
  return;
}
