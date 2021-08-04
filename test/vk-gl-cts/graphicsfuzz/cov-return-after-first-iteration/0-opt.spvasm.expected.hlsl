static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[2];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
cbuffer cbuffer_x_11 : register(b2, space0) {
  uint4 x_11[1];
};

void main_1() {
  int i = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  const int x_42 = asint(x_7[1].x);
  i = x_42;
  while (true) {
    const int x_47 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_49 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    if ((x_47 < x_49)) {
    } else {
      break;
    }
    const int x_52 = i;
    const int x_54 = asint(x_7[1].x);
    if ((x_52 != x_54)) {
      return;
    }
    {
      i = (i + 1);
    }
  }
  const float x_61 = gl_FragCoord.y;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_63 = asfloat(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_61 < x_63)) {
    return;
  }
  const float x_68 = asfloat(x_11[0].y);
  x_GLF_color = float4(float3(1.0f, 1.0f, 1.0f).x, float3(1.0f, 1.0f, 1.0f).y, float3(1.0f, 1.0f, 1.0f).z, x_68);
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
  const main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
