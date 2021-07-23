static float4 position = float4(0.0f, 0.0f, 0.0f, 0.0f);
static uint pos = 0u;
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  gl_Position = position;
  pos = 0u;
  return;
}

struct main_out {
  float4 gl_Position;
  uint pos_1;
};
struct tint_symbol_1 {
  float4 position_param : TEXCOORD0;
};
struct tint_symbol_2 {
  uint pos_1 : TEXCOORD0;
  float4 gl_Position : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 position_param = tint_symbol.position_param;
  position = position_param;
  main_1();
  const main_out tint_symbol_3 = {gl_Position, pos};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.pos_1, tint_symbol_3.gl_Position};
  return tint_symbol_4;
}
