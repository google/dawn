cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b1, space0) {
  uint4 x_11[2];
};

float2 func_() {
  float2 v = float2(0.0f, 0.0f);
  int a = 0;
  float2 indexable[3] = (float2[3])0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_67 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
  v.y = x_67;
  a = 2;
  const float x_70 = asfloat(x_7[1].x);
  const float x_73 = asfloat(x_7[1].x);
  const int x_77 = a;
  const float2 tint_symbol_3[3] = {float2(x_70, x_70), float2(x_73, x_73), v};
  indexable = tint_symbol_3;
  const float2 x_79 = indexable[x_77];
  return x_79;
}

void main_1() {
  const float2 x_40 = func_();
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_43 = asfloat(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_40.y == x_43)) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_49 = asint(x_11[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_52 = asint(x_11[1].x);
    const int x_55 = asint(x_11[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_58 = asint(x_11[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_49), float(x_52), float(x_55), float(x_58));
  } else {
    const int x_62 = asint(x_11[1].x);
    const float x_63 = float(x_62);
    x_GLF_color = float4(x_63, x_63, x_63, x_63);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
