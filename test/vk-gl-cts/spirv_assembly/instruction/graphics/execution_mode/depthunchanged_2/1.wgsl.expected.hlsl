static float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float gl_FragDepth = 0.0f;

void main_1() {
  outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
  gl_FragDepth = 0.699999988f;
  return;
}

struct main_out {
  float4 outColor_1;
  float gl_FragDepth_1;
};
struct tint_symbol {
  float4 outColor_1 : SV_Target0;
  float gl_FragDepth_1 : SV_Depth;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {outColor, gl_FragDepth};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.outColor_1 = inner_result.outColor_1;
  wrapper_result.gl_FragDepth_1 = inner_result.gl_FragDepth_1;
  return wrapper_result;
}
