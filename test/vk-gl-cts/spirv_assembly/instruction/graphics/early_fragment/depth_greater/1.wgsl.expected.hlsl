static float4 color_out = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float gl_FragDepth = 0.0f;

void main_1() {
  color_out = float4(1.0f, 0.0f, 0.0f, 1.0f);
  gl_FragDepth = 0.400000006f;
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

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {color_out, gl_FragDepth};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.color_out_1, tint_symbol_1.gl_FragDepth_1};
  return tint_symbol_2;
}
