#version 310 es


struct modf_result_f32 {
  float fract;
  float whole;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float runtime_in = 1.25f;
  modf_result_f32 res = modf_result_f32(0.25f, 1.0f);
  modf_result_f32 v = modf_result_f32(0.0f, 0.0f);
  v.fract = modf(runtime_in, v.whole);
  res = v;
  res = modf_result_f32(0.25f, 1.0f);
  float tint_symbol_1 = res.fract;
  float whole = res.whole;
}
