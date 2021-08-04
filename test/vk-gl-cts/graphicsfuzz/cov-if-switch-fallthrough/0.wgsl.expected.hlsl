static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[2];
};

void main_1() {
  const float x_31 = asfloat(x_6[1].x);
  x_GLF_color = float4(x_31, x_31, x_31, x_31);
  const float x_34 = gl_FragCoord.y;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_36 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  if ((x_34 >= x_36)) {
    const int x_41 = asint(x_8[1].x);
    switch(x_41) {
      case 0:
      case 16: {
        const uint scalar_offset_1 = ((16u * uint(0))) / 4;
        const int x_45 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
        const float x_46 = float(x_45);
        const float x_47 = float(x_41);
        x_GLF_color = float4(x_46, x_47, x_47, x_46);
        break;
      }
      default: {
        break;
      }
    }
  }
  const int x_50 = asint(x_8[1].x);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_52 = asint(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  if ((x_50 == x_52)) {
    x_GLF_color = float4(x_36, x_36, x_36, x_36);
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
