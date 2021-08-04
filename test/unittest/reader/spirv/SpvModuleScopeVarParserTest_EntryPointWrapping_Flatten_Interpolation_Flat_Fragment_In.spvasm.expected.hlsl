struct S {
  float field0;
  float field1;
};

static float x_1[2] = (float[2])0;
static S x_2 = (S)0;

void main_1() {
  return;
}

struct tint_symbol_1 {
  nointerpolation float x_1_param : TEXCOORD1;
  nointerpolation float x_1_param_1 : TEXCOORD2;
  nointerpolation float x_2_param : TEXCOORD5;
  nointerpolation float x_2_param_1 : TEXCOORD6;
};

void main_inner(float x_1_param, float x_1_param_1, float x_2_param, float x_2_param_1) {
  x_1[0] = x_1_param;
  x_1[1] = x_1_param_1;
  x_2.field0 = x_2_param;
  x_2.field1 = x_2_param_1;
  main_1();
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_1_param, tint_symbol.x_1_param_1, tint_symbol.x_2_param, tint_symbol.x_2_param_1);
  return;
}
