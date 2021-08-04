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
  uint x_1_1;
  uint2 x_2_1;
  int x_3_1;
  int2 x_4_1;
  float x_5_1;
  float2 x_6_1;
  float4 x_8_1;
};
struct tint_symbol {
  uint x_1_1 : TEXCOORD1;
  uint2 x_2_1 : TEXCOORD2;
  int x_3_1 : TEXCOORD3;
  int2 x_4_1 : TEXCOORD4;
  nointerpolation float x_5_1 : TEXCOORD5;
  nointerpolation float2 x_6_1 : TEXCOORD6;
  float4 x_8_1 : SV_Position;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_1, x_2, x_3, x_4, x_5, x_6, x_8};
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
  wrapper_result.x_8_1 = inner_result.x_8_1;
  return wrapper_result;
}
