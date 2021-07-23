static float2 x_3 = float2(0.0f, 0.0f);
static uint x_4 = 0u;
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float x_30 = x_3.x;
  const float x_36 = x_3.y;
  x_4 = (uint((((x_30 + 1.027777791f) * 18.0f) - 1.0f)) + (uint((((x_36 + 1.027777791f) * 18.0f) - 1.0f)) * 36u));
  const float2 x_43 = x_3;
  gl_Position = float4(x_43.x, x_43.y, 0.0f, 1.0f);
  return;
}

struct main_out {
  uint x_4_1;
  float4 gl_Position;
};
struct tint_symbol_1 {
  float2 x_3_param : TEXCOORD0;
};
struct tint_symbol_2 {
  uint x_4_1 : TEXCOORD0;
  float4 gl_Position : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float2 x_3_param = tint_symbol.x_3_param;
  x_3 = x_3_param;
  main_1();
  const main_out tint_symbol_3 = {x_4, gl_Position};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.x_4_1, tint_symbol_3.gl_Position};
  return tint_symbol_4;
}
