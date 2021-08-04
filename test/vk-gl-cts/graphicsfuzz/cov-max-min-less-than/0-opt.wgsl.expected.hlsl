static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};

void main_1() {
  int a = 0;
  const float x_30 = gl_FragCoord.x;
  a = max(1, min(1, int(x_30)));
  if ((a < 2)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_40 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_43 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_46 = asint(x_7[1].x);
    x_GLF_color = float4(1.0f, float(x_40), float(x_43), float(x_46));
  } else {
    const int x_50 = asint(x_7[1].x);
    const float x_51 = float(x_50);
    x_GLF_color = float4(x_51, x_51, x_51, x_51);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
