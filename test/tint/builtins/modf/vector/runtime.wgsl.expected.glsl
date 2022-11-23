#version 310 es

struct modf_result_vec2 {
  vec2 fract;
  vec2 whole;
};

modf_result_vec2 tint_modf(vec2 param_0) {
  modf_result_vec2 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void tint_symbol() {
  vec2 tint_symbol_1 = vec2(1.230000019f, 3.450000048f);
  modf_result_vec2 res = tint_modf(tint_symbol_1);
  vec2 tint_symbol_2 = res.fract;
  vec2 whole = res.whole;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
