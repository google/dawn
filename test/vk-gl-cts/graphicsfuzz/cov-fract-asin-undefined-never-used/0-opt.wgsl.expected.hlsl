static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[2];
};

void main_1() {
  float f0 = 0.0f;
  float f1 = 0.0f;
  f0 = asfloat(0x7f800000u);
  f1 = frac(f0);
  const float x_38 = gl_FragCoord.x;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_40 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
  if ((x_38 > x_40)) {
    const int x_46 = asint(x_10[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_49 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_52 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_55 = asint(x_10[1].x);
    x_GLF_color = float4(float(x_46), float(x_49), float(x_52), float(x_55));
  } else {
    const float x_58 = f1;
    x_GLF_color = float4(x_58, x_58, x_58, x_58);
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
  const main_out tint_symbol_5 = {x_GLF_color};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
