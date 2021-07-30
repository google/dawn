static float2x4 x_1 = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_2 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  return;
}

struct main_out {
  float4 x_2_1;
};
struct tint_symbol_1 {
  float4 x_1_param : TEXCOORD9;
  float4 x_1_param_1 : TEXCOORD10;
};
struct tint_symbol_2 {
  float4 x_2_1 : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 x_1_param = tint_symbol.x_1_param;
  const float4 x_1_param_1 = tint_symbol.x_1_param_1;
  x_1[0] = x_1_param;
  x_1[1] = x_1_param_1;
  main_1();
  const main_out tint_symbol_3 = {x_2};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.x_2_1};
  return tint_symbol_4;
}
