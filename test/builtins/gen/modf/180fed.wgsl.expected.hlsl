struct modf_result {
  float fract;
  float whole;
};
modf_result tint_modf(float param_0) {
  float whole;
  float fract = modf(param_0, whole);
  modf_result result = {fract, whole};
  return result;
}

void modf_180fed() {
  modf_result res = tint_modf(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_180fed();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_180fed();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_180fed();
  return;
}
