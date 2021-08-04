static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[2];
};

void main_1() {
  int a = 0;
  float b = 0.0f;
  const float x_39 = gl_FragCoord.y;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_41 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
  a = ((x_39 >= x_41) ? 0 : 2);
  const float x_45 = asfloat(x_7[1].x);
  const float x_47 = asfloat(x_7[2].x);
  const float x_49 = asfloat(x_7[3].x);
  b = float3(x_45, x_47, x_49)[a];
  const float x_53 = b;
  const float x_55 = asfloat(x_7[1].x);
  if ((x_53 == x_55)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_61 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_64 = asint(x_10[1].x);
    const int x_67 = asint(x_10[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_70 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(float(x_61), float(x_64), float(x_67), float(x_70));
  } else {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_74 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_75 = float(x_74);
    x_GLF_color = float4(x_75, x_75, x_75, x_75);
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
