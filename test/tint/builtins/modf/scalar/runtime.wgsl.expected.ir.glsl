#version 310 es


struct modf_result_f32 {
  float fract;
  float whole;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float tint_symbol_1 = 1.25f;
  modf_result_f32 v = modf_result_f32(0.0f, 0.0f);
  v.fract = modf(tint_symbol_1, v.whole);
  modf_result_f32 res = v;
  float tint_symbol_2 = res.fract;
  float whole = res.whole;
}
