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

main_out main_inner(float4 frag_color_param) {
  frag_color = frag_color_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.frag_color_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
