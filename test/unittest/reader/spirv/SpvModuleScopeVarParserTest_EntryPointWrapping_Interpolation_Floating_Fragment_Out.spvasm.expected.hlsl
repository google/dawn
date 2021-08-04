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

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_1, x_2, x_3, x_4, x_5, x_6};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_1_1 = inner_result.x_1_1;
  wrapper_result.x_2_1 = inner_result.x_2_1;
  wrapper_result.x_3_1 = inner_result.x_3_1;
  wrapper_result.x_4_1 = inner_result.x_4_1;
  wrapper_result.x_5_1 = inner_result.x_5_1;
  wrapper_result.x_6_1 = inner_result.x_6_1;
  return wrapper_result;
}
