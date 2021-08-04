static float x_1 = 0.0f;
static float x_2 = 0.0f;
static float x_3 = 0.0f;
static float x_4 = 0.0f;
static float x_5 = 0.0f;
static float x_6 = 0.0f;

void main_1() {
  return;
}

struct tint_symbol_1 {
  float x_1_param : TEXCOORD1;
  linear centroid float x_2_param : TEXCOORD2;
  linear sample float x_3_param : TEXCOORD3;
  noperspective float x_4_param : TEXCOORD4;
  noperspective centroid float x_5_param : TEXCOORD5;
  noperspective sample float x_6_param : TEXCOORD6;
};

void main_inner(float x_1_param, float x_2_param, float x_3_param, float x_4_param, float x_5_param, float x_6_param) {
  x_1 = x_1_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  x_4 = x_4_param;
  x_5 = x_5_param;
  x_6 = x_6_param;
  main_1();
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_1_param, tint_symbol.x_2_param, tint_symbol.x_3_param, tint_symbol.x_4_param, tint_symbol.x_5_param, tint_symbol.x_6_param);
  return;
}
