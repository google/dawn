cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[3];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b1, space0) {
  uint4 x_11[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int arr[3] = (int[3])0;
  int a = 0;
  int b = 0;
  int c = 0;
  const int x_40 = asint(x_7[1].x);
  const int x_42 = asint(x_7[1].x);
  const int x_44 = asint(x_7[1].x);
  const int tint_symbol_5[3] = {x_40, x_42, x_44};
  arr = tint_symbol_5;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_47 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
  const int x_49 = arr[x_47];
  a = x_49;
  b = (a - 1);
  const float x_53 = gl_FragCoord.x;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_55 = asfloat(x_11[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_53 < x_55)) {
    b = (b + 1);
  }
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_62 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  c = x_62;
  const int x_63 = c;
  const int x_65 = asint(x_7[1].x);
  const int x_67 = asint(x_7[2].x);
  arr[clamp(x_63, x_65, x_67)] = b;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_72 = asint(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const int x_74 = arr[x_72];
  const int x_77 = asint(x_7[1].x);
  const int x_79 = arr[x_77];
  const int x_82 = asint(x_7[1].x);
  const int x_84 = arr[x_82];
  const int x_87 = asint(x_7[2].x);
  const int x_89 = arr[x_87];
  x_GLF_color = float4(float(x_74), float(x_79), float(x_84), float(x_89));
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
