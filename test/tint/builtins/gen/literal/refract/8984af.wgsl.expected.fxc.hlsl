SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void refract_8984af() {
  vector<float16_t, 3> res = (float16_t(-5.0h)).xxx;
  prevent_dce.Store<vector<float16_t, 3> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  refract_8984af();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  refract_8984af();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  refract_8984af();
  return;
}
