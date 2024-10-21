#version 310 es


struct frexp_result_f32 {
  float fract;
  int exp;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_result_f32 res = frexp_result_f32(0.61500000953674316406f, 1);
  int tint_symbol_1 = res.exp;
  float tint_symbol_2 = res.fract;
}
