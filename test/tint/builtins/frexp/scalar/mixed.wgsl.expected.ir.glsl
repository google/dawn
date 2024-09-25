#version 310 es


struct frexp_result_f32 {
  float fract;
  int exp;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float runtime_in = 1.25f;
  frexp_result_f32 res = frexp_result_f32(0.625f, 1);
  frexp_result_f32 v = frexp_result_f32(0.0f, 0);
  v.fract = frexp(runtime_in, v.exp);
  res = v;
  res = frexp_result_f32(0.625f, 1);
  float tint_symbol_1 = res.fract;
  int tint_symbol_2 = res.exp;
}
