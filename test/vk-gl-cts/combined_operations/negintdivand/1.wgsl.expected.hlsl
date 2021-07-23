static float4 frag_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 color_out = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int2 iv = int2(0, 0);
  const float4 x_28 = frag_color;
  iv = int2((float2(x_28.x, x_28.y) * 256.0f));
  const int x_33 = iv.y;
  if ((((x_33 / 2) & 64) == 64)) {
    color_out = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    color_out = float4(0.0f, 1.0f, 1.0f, 1.0f);
  }
  return;
}

struct main_out {
  float4 color_out_1;
};
struct tint_symbol_1 {
  float4 frag_color_param : TEXCOORD1;
};
struct tint_symbol_2 {
  float4 color_out_1 : SV_Target0;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 frag_color_param = tint_symbol.frag_color_param;
  frag_color = frag_color_param;
  main_1();
  const main_out tint_symbol_3 = {color_out};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.color_out_1};
  return tint_symbol_4;
}
