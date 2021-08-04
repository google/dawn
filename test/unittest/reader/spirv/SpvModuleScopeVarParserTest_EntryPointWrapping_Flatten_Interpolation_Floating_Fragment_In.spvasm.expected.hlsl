struct S {
  float field0;
  float field1;
  float field2;
  float field3;
  float field4;
  float field5;
};

static S x_1 = (S)0;

void main_1() {
  return;
}

struct tint_symbol_1 {
  float x_1_param : TEXCOORD1;
  linear centroid float x_1_param_1 : TEXCOORD2;
  linear sample float x_1_param_2 : TEXCOORD3;
  noperspective float x_1_param_3 : TEXCOORD4;
  noperspective centroid float x_1_param_4 : TEXCOORD5;
  noperspective sample float x_1_param_5 : TEXCOORD6;
};

void main_inner(float x_1_param, float x_1_param_1, float x_1_param_2, float x_1_param_3, float x_1_param_4, float x_1_param_5) {
  x_1.field0 = x_1_param;
  x_1.field1 = x_1_param_1;
  x_1.field2 = x_1_param_2;
  x_1.field3 = x_1_param_3;
  x_1.field4 = x_1_param_4;
  x_1.field5 = x_1_param_5;
  main_1();
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_1_param, tint_symbol.x_1_param_1, tint_symbol.x_1_param_2, tint_symbol.x_1_param_3, tint_symbol.x_1_param_4, tint_symbol.x_1_param_5);
  return;
}
