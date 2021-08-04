static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  const float x_32 = gl_FragCoord.x;
  const int x_35 = asint(x_7[1].x);
  a = ((int(x_32) < x_35) ? 0 : -1);
  i = 0;
  {
    for(; (i < 5); i = (i + 1)) {
      a = (a / 2);
    }
  }
  if ((a == 0)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_55 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    const int x_58 = asint(x_7[1].x);
    const int x_61 = asint(x_7[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_64 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color = float4(float(x_55), float(x_58), float(x_61), float(x_64));
  } else {
    const int x_68 = asint(x_7[1].x);
    const float x_69 = float(x_68);
    x_GLF_color = float4(x_69, x_69, x_69, x_69);
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
