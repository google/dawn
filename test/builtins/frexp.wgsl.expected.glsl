#version 310 es

struct frexp_result {
  float sig;
  int exp;
};

frexp_result tint_frexp(float param_0) {
  frexp_result result;
  result.sig = frexp(param_0, result.exp);
  return result;
}


void tint_symbol() {
  frexp_result res = tint_frexp(1.230000019f);
  int tint_symbol_1 = res.exp;
  float sig = res.sig;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
