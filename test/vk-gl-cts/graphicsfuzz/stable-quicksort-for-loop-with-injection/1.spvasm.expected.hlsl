static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 frag_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  x_GLF_color = frag_color;
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 frag_color_param : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 frag_color_param = tint_symbol.frag_color_param;
  frag_color = frag_color_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.x_GLF_color_1};
  return tint_symbol_4;
}
