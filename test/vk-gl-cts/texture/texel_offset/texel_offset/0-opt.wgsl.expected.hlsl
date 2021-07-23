static float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float x_19 = gl_FragCoord.x;
  const float x_23 = gl_FragCoord.y;
  result = float4((floor(x_19) / 255.0f), (floor(x_23) / 255.0f), 0.0f, 0.0f);
  return;
}

struct main_out {
  float4 result_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 result_1 : SV_Target0;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 gl_FragCoord_param = tint_symbol.gl_FragCoord_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {result};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.result_1};
  return tint_symbol_4;
}
