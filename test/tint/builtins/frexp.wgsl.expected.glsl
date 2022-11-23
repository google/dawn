#version 310 es

struct frexp_result {
  float fract;
  int exp;
};


void tint_symbol() {
  frexp_result res = frexp_result(0.61500001f, 1);
  int tint_symbol_1 = res.exp;
  float tint_symbol_2 = res.fract;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
