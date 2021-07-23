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

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 color_in_param = tint_symbol.color_in_param;
  color_in = color_in_param;
  main_1();
  const main_out tint_symbol_3 = {color_out};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.color_out_1};
  return tint_symbol_4;
}
