static float4 position = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float4 x_22 = position;
  const float2 x_23 = float2(x_22.x, x_22.y);
  gl_Position = float4(x_23.x, x_23.y, 0.400000006f, 1.0f);
  return;
}

struct main_out {
  float4 gl_Position;
};
struct tint_symbol_1 {
  float4 position_param : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 gl_Position : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 position_param = tint_symbol.position_param;
  position = position_param;
  main_1();
  const main_out tint_symbol_3 = {gl_Position};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.gl_Position};
  return tint_symbol_4;
}
