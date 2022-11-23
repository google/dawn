#version 310 es

struct modf_result_f32 {
  float fract;
  float whole;
};

modf_result_f32 tint_modf(float param_0) {
  modf_result_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void tint_symbol() {
  float runtime_in = 1.25f;
  modf_result_f32 res = modf_result_f32(0.25f, 1.0f);
  res = tint_modf(runtime_in);
  res = modf_result_f32(0.25f, 1.0f);
  float tint_symbol_1 = res.fract;
  float whole = res.whole;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
