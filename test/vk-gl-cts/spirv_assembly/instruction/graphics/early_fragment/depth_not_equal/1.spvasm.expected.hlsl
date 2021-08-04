static float4 color_out = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float gl_FragDepth = 0.0f;

void main_1() {
  color_out = float4(1.0f, 0.0f, 0.0f, 1.0f);
  gl_FragDepth = 0.5f;
  return;
}

struct main_out {
  float4 color_out_1;
  float gl_FragDepth_1;
};
struct tint_symbol {
  float4 color_out_1 : SV_Target0;
  float gl_FragDepth_1 : SV_Depth;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {color_out, gl_FragDepth};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.color_out_1 = inner_result.color_out_1;
  wrapper_result.gl_FragDepth_1 = inner_result.gl_FragDepth_1;
  return wrapper_result;
}
