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

main_out main_inner(float4 frag_color_param) {
  frag_color = frag_color_param;
  main_1();
  const main_out tint_symbol_3 = {color_out};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.frag_color_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.color_out_1 = inner_result.color_out_1;
  return wrapper_result;
}
