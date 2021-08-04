static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_14 : register(b1, space0) {
  uint4 x_14[2];
};

float f1_f1_(inout float a) {
  int b = 0;
  float c = 0.0f;
  b = 8;
  const float x_71 = gl_FragCoord.y;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_73 = asfloat(x_9[scalar_offset / 4][scalar_offset % 4]);
  if ((x_71 >= x_73)) {
    b = (b + 1);
    b = (b + 1);
  }
  const float x_81 = a;
  const float x_83 = asfloat(x_9[1].x);
  if ((x_81 < x_83)) {
    const float x_88 = asfloat(x_9[1].x);
    return x_88;
  }
  c = float(clamp(b, 0, 2));
  return c;
}

void main_1() {
  float a_1 = 0.0f;
  float param = 0.0f;
  const float x_43 = asfloat(x_9[1].x);
  param = x_43;
  const float x_44 = f1_f1_(param);
  a_1 = x_44;
  const float x_45 = a_1;
  const float x_47 = asfloat(x_9[2].x);
  if ((x_45 == x_47)) {
    const int x_53 = asint(x_14[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_56 = asint(x_14[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_59 = asint(x_14[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_62 = asint(x_14[1].x);
    x_GLF_color = float4(float(x_53), float(x_56), float(x_59), float(x_62));
  } else {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_66 = asint(x_14[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_67 = float(x_66);
    x_GLF_color = float4(x_67, x_67, x_67, x_67);
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
