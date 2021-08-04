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

main_out main_inner(float4 position_param) {
  position = position_param;
  main_1();
  const main_out tint_symbol_3 = {gl_Position, pos};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.position_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.gl_Position = inner_result.gl_Position;
  wrapper_result.pos_1 = inner_result.pos_1;
  return wrapper_result;
}
