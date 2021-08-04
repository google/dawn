struct S_1 {
  float4 field0;
};

static float4 x_3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};

void main_1() {
  const float4 x_20 = asfloat(x_5[0]);
  S_1 x_21_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  x_21_1.field0 = x_20;
  x_3 = x_21_1.field0;
  return;
}

struct main_out {
  float4 x_3_1;
};
struct tint_symbol {
  float4 x_3_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_2 = {x_3};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_3_1 = inner_result.x_3_1;
  return wrapper_result;
}
