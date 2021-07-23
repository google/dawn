static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float x_20 = gl_FragCoord.x;
  const float x_23 = gl_FragCoord.x;
  const float x_26 = gl_FragCoord.y;
  const float x_32 = gl_FragCoord.y;
  x_GLF_color = float4((x_20 * 0.00390625f), (float((int(x_23) ^ int(x_26))) * 0.00390625f), (x_32 * 0.00390625f), 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 gl_FragCoord_param = tint_symbol.gl_FragCoord_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.x_GLF_color_1};
  return tint_symbol_4;
}
