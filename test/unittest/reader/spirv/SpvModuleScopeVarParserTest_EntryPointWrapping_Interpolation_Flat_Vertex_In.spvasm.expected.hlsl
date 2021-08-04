static uint x_1 = 0u;
static uint2 x_2 = uint2(0u, 0u);
static int x_3 = 0;
static int2 x_4 = int2(0, 0);
static float x_5 = 0.0f;
static float2 x_6 = float2(0.0f, 0.0f);
static float4 x_8 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  return;
}

struct main_out {
  float4 x_8_1;
};
struct tint_symbol_1 {
  uint x_1_param : TEXCOORD1;
  uint2 x_2_param : TEXCOORD2;
  int x_3_param : TEXCOORD3;
  int2 x_4_param : TEXCOORD4;
  nointerpolation float x_5_param : TEXCOORD5;
  nointerpolation float2 x_6_param : TEXCOORD6;
};
struct tint_symbol_2 {
  float4 x_8_1 : SV_Position;
};

main_out main_inner(uint x_1_param, uint2 x_2_param, int x_3_param, int2 x_4_param, float x_5_param, float2 x_6_param) {
  x_1 = x_1_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  x_4 = x_4_param;
  x_5 = x_5_param;
  x_6 = x_6_param;
  main_1();
  const main_out tint_symbol_3 = {x_8};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.x_1_param, tint_symbol.x_2_param, tint_symbol.x_3_param, tint_symbol.x_4_param, tint_symbol.x_5_param, tint_symbol.x_6_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_8_1 = inner_result.x_8_1;
  return wrapper_result;
}
