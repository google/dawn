static float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float gl_FragDepth = 0.0f;
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float x_20 = gl_FragCoord.z;
  gl_FragDepth = x_20;
  return;
}

struct main_out {
  float4 outColor_1;
  float gl_FragDepth_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 outColor_1 : SV_Target0;
  float gl_FragDepth_1 : SV_Depth;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 gl_FragCoord_param = tint_symbol.gl_FragCoord_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {outColor, gl_FragDepth};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.outColor_1, tint_symbol_3.gl_FragDepth_1};
  return tint_symbol_4;
}
