#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};


void tint_symbol() {
  frexp_result_f32 res = frexp_result_f32(0.625f, 1);
  float tint_symbol_2 = res.fract;
  int tint_symbol_3 = res.exp;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
