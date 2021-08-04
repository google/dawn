static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
cbuffer cbuffer_x_11 : register(b1, space0) {
  uint4 x_11[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int f1_() {
  int a = 0;
  int i = 0;
  a = 256;
  const float x_65 = gl_FragCoord.y;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_67 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
  if ((x_65 > x_67)) {
    a = (a + 1);
  }
  i = countbits(a);
  const int x_75 = i;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_77 = asint(x_11[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_75 < x_77)) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_82 = asint(x_11[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    return x_82;
  }
  return i;
}

void main_1() {
  int a_1 = 0;
  const int x_38 = f1_();
  a_1 = x_38;
  const int x_39 = a_1;
  const int x_41 = asint(x_11[2].x);
  if ((x_39 == x_41)) {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_47 = asint(x_11[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_50 = asint(x_11[1].x);
    const int x_53 = asint(x_11[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_56 = asint(x_11[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_GLF_color = float4(float(x_47), float(x_50), float(x_53), float(x_56));
  } else {
    const int x_60 = asint(x_11[1].x);
    const float x_61 = float(x_60);
    x_GLF_color = float4(x_61, x_61, x_61, x_61);
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
