static float4 position = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 frag_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  gl_Position = position;
  const float4 x_27 = asfloat(x_8[0]);
  frag_color = x_27;
  return;
}

struct main_out {
  float4 gl_Position;
  float4 frag_color_1;
};
struct tint_symbol_1 {
  float4 position_param : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 frag_color_1 : TEXCOORD0;
  float4 gl_Position : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 position_param = tint_symbol.position_param;
  position = position_param;
  main_1();
  const main_out tint_symbol_3 = {gl_Position, frag_color};
  const tint_symbol_2 tint_symbol_5 = {tint_symbol_3.frag_color_1, tint_symbol_3.gl_Position};
  return tint_symbol_5;
}
