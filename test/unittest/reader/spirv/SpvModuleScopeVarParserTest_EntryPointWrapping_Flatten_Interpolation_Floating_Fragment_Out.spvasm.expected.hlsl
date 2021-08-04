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

struct main_out {
  float x_1_1;
  float x_1_2;
  float x_1_3;
  float x_1_4;
  float x_1_5;
  float x_1_6;
};
struct tint_symbol {
  float x_1_1 : SV_Target1;
  linear centroid float x_1_2 : SV_Target2;
  linear sample float x_1_3 : SV_Target3;
  noperspective float x_1_4 : SV_Target4;
  noperspective centroid float x_1_5 : SV_Target5;
  noperspective sample float x_1_6 : SV_Target6;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_1.field0, x_1.field1, x_1.field2, x_1.field3, x_1.field4, x_1.field5};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_1_1 = inner_result.x_1_1;
  wrapper_result.x_1_2 = inner_result.x_1_2;
  wrapper_result.x_1_3 = inner_result.x_1_3;
  wrapper_result.x_1_4 = inner_result.x_1_4;
  wrapper_result.x_1_5 = inner_result.x_1_5;
  wrapper_result.x_1_6 = inner_result.x_1_6;
  return wrapper_result;
}
