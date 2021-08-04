static float4 color_out = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 color_in = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  color_out = color_in;
  return;
}

struct main_out {
  float4 color_out_1;
};
struct tint_symbol_1 {
  float4 color_in_param : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 color_out_1 : SV_Target0;
};

main_out main_inner(float4 color_in_param) {
  color_in = color_in_param;
  main_1();
  const main_out tint_symbol_3 = {color_out};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.color_in_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.color_out_1 = inner_result.color_out_1;
  return wrapper_result;
}
