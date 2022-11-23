#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};

frexp_result_f32 tint_frexp(float param_0) {
  frexp_result_f32 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}


void tint_symbol() {
  float runtime_in = 1.25f;
  frexp_result_f32 res = frexp_result_f32(0.625f, 1);
  res = tint_frexp(runtime_in);
  res = frexp_result_f32(0.625f, 1);
  float tint_symbol_1 = res.fract;
  int tint_symbol_2 = res.exp;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
