SKIP: FAILED

cbuffer cbuffer_x_8 : register(b2, space0) {
  uint4 x_8[2];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
cbuffer cbuffer_x_12 : register(b1, space0) {
  uint4 x_12[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  int b = 0;
  a = 0u;
  const int x_41 = asint(x_8[1].x);
  b = x_41;
  const float x_43 = gl_FragCoord.x;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_45 = asfloat(x_10[scalar_offset / 4][scalar_offset % 4]);
  if ((x_43 < x_45)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const uint x_50 = x_12[scalar_offset_1 / 4][scalar_offset_1 % 4];
    b = asint((x_50 % a));
  }
  const int x_54 = b;
  const int x_56 = asint(x_8[1].x);
  if ((x_54 == x_56)) {
    const int x_62 = asint(x_8[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_65 = asint(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_68 = asint(x_8[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_71 = asint(x_8[1].x);
    x_GLF_color = float4(float(x_62), float(x_65), float(x_68), float(x_71));
  } else {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_75 = asint(x_8[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_76 = float(x_75);
    x_GLF_color = float4(x_76, x_76, x_76, x_76);
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
  const main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x0000026AFDC08170(25,16-23): error X4010: Unsigned integer divide by zero

