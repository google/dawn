static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[2];
};

void main_1() {
  int2 v = int2(0, 0);
  const float x_39 = gl_FragCoord.y;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_41 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  if ((x_39 < x_41)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_47 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_48 = float(x_47);
    x_GLF_color = float4(x_48, x_48, x_48, x_48);
  } else {
    const float4 x_50 = gl_FragCoord;
    const float x_53 = asfloat(x_6[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_55 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const float x_59 = asfloat(x_6[2].x);
    v = int2(((float2(x_50.x, x_50.y) - float2(x_53, x_55)) * x_59));
    const float x_63 = asfloat(x_6[1].x);
    const int x_65 = v.y;
    const int x_67 = asint(x_8[1].x);
    const int x_70 = asint(x_8[1].x);
    const int x_74 = v.x;
    const int x_76 = asint(x_8[1].x);
    const float x_80 = asfloat(x_6[1].x);
    x_GLF_color = float4(x_63, float(((x_65 - x_67) & x_70)), float((x_74 & x_76)), x_80);
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
