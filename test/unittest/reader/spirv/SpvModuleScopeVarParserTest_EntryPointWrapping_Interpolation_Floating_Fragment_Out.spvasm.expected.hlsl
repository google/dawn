static float x_1 = 0.0f;
static float x_2 = 0.0f;
static float x_3 = 0.0f;
static float x_4 = 0.0f;
static float x_5 = 0.0f;
static float x_6 = 0.0f;

void main_1() {
  return;
}

struct main_out {
  float x_1_1;
  float x_2_1;
  float x_3_1;
  float x_4_1;
  float x_5_1;
  float x_6_1;
};
struct tint_symbol {
  float x_1_1 : SV_Target1;
  linear centroid float x_2_1 : SV_Target2;
  linear sample float x_3_1 : SV_Target3;
  noperspective float x_4_1 : SV_Target4;
  noperspective centroid float x_5_1 : SV_Target5;
  noperspective sample float x_6_1 : SV_Target6;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_1, x_2, x_3, x_4, x_5, x_6};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.x_1_1, tint_symbol_1.x_2_1, tint_symbol_1.x_3_1, tint_symbol_1.x_4_1, tint_symbol_1.x_5_1, tint_symbol_1.x_6_1};
  return tint_symbol_2;
}
