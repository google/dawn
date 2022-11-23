#version 310 es

struct modf_result {
  float fract;
  float whole;
};

modf_result tint_modf(float param_0) {
  modf_result result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void tint_symbol() {
  float tint_symbol_1 = 1.25f;
  modf_result res = tint_modf(tint_symbol_1);
  float tint_symbol_2 = res.fract;
  float whole = res.whole;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
