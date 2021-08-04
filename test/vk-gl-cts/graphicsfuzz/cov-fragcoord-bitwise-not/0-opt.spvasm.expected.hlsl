static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  const float x_28 = gl_FragCoord.x;
  a = int(x_28);
  if ((~(a) < 0)) {
    const int x_36 = asint(x_7[1].x);
    a = x_36;
  }
  const int x_37 = a;
  const int x_39 = asint(x_7[1].x);
  if ((x_37 == x_39)) {
    const int x_45 = asint(x_7[1].x);
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_48 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_51 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_54 = asint(x_7[1].x);
    x_GLF_color = float4(float(x_45), float(x_48), float(x_51), float(x_54));
  } else {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_58 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const float x_59 = float(x_58);
    x_GLF_color = float4(x_59, x_59, x_59, x_59);
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
